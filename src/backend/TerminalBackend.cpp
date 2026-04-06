#include "TerminalBackend.h"
#include <QDebug>

#ifdef Q_OS_MACOS
#include <util.h>
#elif defined(Q_OS_LINUX)
#include <pty.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <pwd.h>

// ── FileDescriptor RAII ─────────────────────────────────────────────

FileDescriptor::~FileDescriptor() {
    if (m_fd >= 0) ::close(m_fd);
}

FileDescriptor::FileDescriptor(FileDescriptor &&other) noexcept
    : m_fd(std::exchange(other.m_fd, -1)) {}

FileDescriptor &FileDescriptor::operator=(FileDescriptor &&other) noexcept {
    if (this != &other) {
        reset();
        m_fd = std::exchange(other.m_fd, -1);
    }
    return *this;
}

int FileDescriptor::release() {
    return std::exchange(m_fd, -1);
}

void FileDescriptor::reset(int fd) {
    if (m_fd >= 0) ::close(m_fd);
    m_fd = fd;
}

// ── libvterm callbacks (C linkage style, static) ────────────────────

static int screen_damage(VTermRect /*rect*/, void *user) {
    auto *backend = static_cast<TerminalBackend *>(user);
    emit backend->screenUpdated();
    return 1;
}

static int screen_movecursor(VTermPos pos, VTermPos /*oldpos*/, int /*visible*/, void *user) {
    auto *backend = static_cast<TerminalBackend *>(user);
    backend->setCursorPos(pos.row, pos.col);
    return 1;
}

static int screen_settermprop(VTermProp /*prop*/, VTermValue * /*val*/, void * /*user*/) {
    return 1;
}

// NOLINTBEGIN(readability-redundant-member-init)
static const VTermScreenCallbacks screen_callbacks = {
    .damage       = screen_damage,
    .moverect     = nullptr,
    .movecursor   = screen_movecursor,
    .settermprop  = screen_settermprop,
    .bell         = nullptr,
    .resize       = nullptr,
    .sb_pushline  = nullptr,
    .sb_popline   = nullptr,
};
// NOLINTEND(readability-redundant-member-init)

static void on_vterm_output(const char *s, size_t len, void *user) {
    auto *backend = static_cast<TerminalBackend *>(user);
    backend->writeToPty(QByteArray(s, static_cast<qsizetype>(len)));
}

// ── TerminalBackend ─────────────────────────────────────────────────

TerminalBackend::TerminalBackend(QObject *parent)
    : QObject(parent)
{
    setupVTerm();
}

TerminalBackend::~TerminalBackend() {
    cleanupChild();
}

void TerminalBackend::cleanupChild() {
    if (m_notifier) {
        m_notifier->setEnabled(false);
        delete m_notifier;
        m_notifier = nullptr;
    }

    m_masterFd.reset(); // Close PTY master first so child gets HUP

    if (m_childPid > 0) {
        int status = 0;
        if (waitpid(m_childPid, &status, WNOHANG) == 0) {
            // Child still running — send SIGHUP then reap
            kill(m_childPid, SIGHUP);
            waitpid(m_childPid, &status, 0);
        }
        m_childPid = -1;
    }
}

// Helper: convert a VTermColor to an RGB hex string like "#rrggbb".
// Uses the screen's palette to resolve indexed colors.
static QString vtermColorToHex(const VTermScreen *screen, VTermColor col) {
    if (VTERM_COLOR_IS_INDEXED(&col)) {
        vterm_screen_convert_color_to_rgb(screen, &col);
    }
    return QStringLiteral("#%1%2%3")
        .arg(col.rgb.red,   2, 16, QLatin1Char('0'))
        .arg(col.rgb.green, 2, 16, QLatin1Char('0'))
        .arg(col.rgb.blue,  2, 16, QLatin1Char('0'));
}

// Helper: append a QChar for a uint32_t codepoint.
static void appendCodepoint(QString &out, uint32_t c) {
    if (QChar::requiresSurrogates(c)) {
        out.append(QChar::highSurrogate(c));
        out.append(QChar::lowSurrogate(c));
    } else {
        out.append(QChar(static_cast<char16_t>(c)));
    }
}

// Helper: extract the character(s) from a cell into a string, HTML-escaped.
static void appendCellText(QString &out, const VTermScreenCell &cell) {
    if (cell.chars[0] == 0) {
        out.append(u' ');
        return;
    }
    for (int i = 0; i < VTERM_MAX_CHARS_PER_CELL && cell.chars[i]; ++i) {
        const auto c = cell.chars[i];
        // HTML-escape special characters
        if (c == '<')       out.append(QStringLiteral("&lt;"));
        else if (c == '>')  out.append(QStringLiteral("&gt;"));
        else if (c == '&')  out.append(QStringLiteral("&amp;"));
        else                appendCodepoint(out, c);
    }
}

void TerminalBackend::setupVTerm() {
    m_vt.reset(vterm_new(m_rows, m_cols));
    vterm_set_utf8(m_vt.get(), 1);
    vterm_output_set_callback(m_vt.get(), on_vterm_output, this);

    m_vts = vterm_obtain_screen(m_vt.get());
    vterm_screen_set_callbacks(m_vts, &screen_callbacks, this);

    // Set default fg/bg to match Neo-Kitsch theme
    VTermColor defaultFg, defaultBg;
    vterm_color_rgb(&defaultFg, 0xe8, 0xdf, 0xc0);  // Style.textPrimary
    vterm_color_rgb(&defaultBg, 0x0d, 0x0b, 0x0e);  // Style.backgroundColor
    vterm_screen_set_default_colors(m_vts, &defaultFg, &defaultBg);

    vterm_screen_reset(m_vts, 1);
}

void TerminalBackend::start(const QString &shell) {
    // Resolve shell path
    QString shellToUse = shell;
    if (shellToUse.isEmpty()) {
        if (const auto *pw = getpwuid(getuid()); pw && pw->pw_shell) {
            shellToUse = QString::fromLocal8Bit(pw->pw_shell);
        } else {
            shellToUse = QStringLiteral("/bin/sh");
        }
    }

    qDebug() << "Starting shell:" << shellToUse << "rows:" << m_rows << "cols:" << m_cols;

    struct winsize ws {};
    ws.ws_row = static_cast<unsigned short>(m_rows);
    ws.ws_col = static_cast<unsigned short>(m_cols);

    int masterFd = -1;
    m_childPid = forkpty(&masterFd, nullptr, nullptr, &ws);

    if (m_childPid == 0) {
        // ── Child process ──
        setenv("TERM", "xterm-256color", 1);
        const QByteArray shellPath = shellToUse.toLocal8Bit();
        const QByteArray shellName = QByteArray("-") + shellToUse.section('/', -1).toLocal8Bit();
        execl(shellPath.constData(), shellName.constData(), nullptr);
        perror("execl");
        _exit(1);
    }

    if (m_childPid < 0) {
        qWarning() << "forkpty failed:" << strerror(errno);
        return;
    }

    // ── Parent process ──
    m_masterFd.reset(masterFd);
    fcntl(m_masterFd.get(), F_SETFL, O_NONBLOCK);

    m_notifier = new QSocketNotifier(m_masterFd.get(), QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated,
            this, &TerminalBackend::onPtyReadReady);

    qDebug() << "Shell started: PID" << m_childPid << "FD" << m_masterFd.get();
}

void TerminalBackend::onPtyReadReady() {
    char buffer[4096];
    const auto bytesRead = read(m_masterFd.get(), buffer, sizeof(buffer));

    if (bytesRead > 0) {
        vterm_input_write(m_vt.get(), buffer, static_cast<size_t>(bytesRead));
        vterm_screen_flush_damage(m_vts);
        return;
    }

    if (bytesRead == 0 || (bytesRead < 0 && errno != EAGAIN)) {
        m_notifier->setEnabled(false);
        qDebug() << "PTY closed";
    }
}

void TerminalBackend::sendInput(const QString &input) {
    if (!m_masterFd.isValid()) return;

    const QByteArray data = input.toUtf8();
    const auto written = write(m_masterFd.get(), data.constData(), static_cast<size_t>(data.size()));
    if (written < 0) {
        qWarning() << "PTY write failed:" << strerror(errno);
    }
}

void TerminalBackend::writeToPty(const QByteArray &data) {
    if (!m_masterFd.isValid()) return;

    const auto written = write(m_masterFd.get(), data.constData(), static_cast<size_t>(data.size()));
    if (written < 0) {
        qWarning() << "PTY write (vterm output) failed:" << strerror(errno);
    }
}

void TerminalBackend::setRows(int rows) {
    if (rows <= 0 || m_rows == rows) return;
    m_rows = rows;
    if (m_vt) vterm_set_size(m_vt.get(), m_rows, m_cols);
    resizePty();
    emit rowsChanged();
}

void TerminalBackend::setCols(int cols) {
    if (cols <= 0 || m_cols == cols) return;
    m_cols = cols;
    if (m_vt) vterm_set_size(m_vt.get(), m_rows, m_cols);
    resizePty();
    emit colsChanged();
}

void TerminalBackend::resize(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return;
    const bool changed = (m_rows != rows || m_cols != cols);
    if (!changed) return;

    const bool rowsChanged = (m_rows != rows);
    const bool colsChanged = (m_cols != cols);
    m_rows = rows;
    m_cols = cols;

    if (m_vt) vterm_set_size(m_vt.get(), m_rows, m_cols);
    resizePty();

    if (rowsChanged) emit this->rowsChanged();
    if (colsChanged) emit this->colsChanged();
}

void TerminalBackend::resizePty() {
    if (!m_masterFd.isValid()) return;

    struct winsize ws {};
    ws.ws_row = static_cast<unsigned short>(m_rows);
    ws.ws_col = static_cast<unsigned short>(m_cols);
    ioctl(m_masterFd.get(), TIOCSWINSZ, &ws);
}

void TerminalBackend::setCursorPos(int row, int col) {
    if (m_cursorRow == row && m_cursorCol == col) return;
    m_cursorRow = row;
    m_cursorCol = col;
    emit cursorMoved();
}

QString TerminalBackend::getLineText(int row) const {
    if (row < 0 || row >= m_rows) return {};

    QString line;
    line.reserve(m_cols);

    for (int col = 0; col < m_cols; ++col) {
        VTermScreenCell cell;
        const VTermPos pos = { row, col };
        vterm_screen_get_cell(m_vts, pos, &cell);

        if (cell.chars[0] == 0) {
            line.append(u' ');
        } else {
            for (int i = 0; i < VTERM_MAX_CHARS_PER_CELL && cell.chars[i]; ++i) {
                appendCodepoint(line, cell.chars[i]);
            }
        }

        // Skip continuation cells for double-width characters
        if (cell.width > 1) {
            col += cell.width - 1;
        }
    }

    return line;
}

QString TerminalBackend::getLineHtml(int row) const {
    if (row < 0 || row >= m_rows) return {};

    // Pre-allocate generously: typical line ~cols chars + markup overhead
    QString html;
    html.reserve(m_cols * 6);

    // Track current span attributes to batch consecutive cells
    QString curFg, curBg;
    bool curBold = false;
    bool curItalic = false;
    bool curUnderline = false;
    bool spanOpen = false;

    auto closeSpan = [&]() {
        if (spanOpen) {
            html.append(QStringLiteral("</span>"));
            spanOpen = false;
        }
    };

    for (int col = 0; col < m_cols; ++col) {
        VTermScreenCell cell;
        const VTermPos pos = { row, col };
        vterm_screen_get_cell(m_vts, pos, &cell);

        // Resolve colors
        const bool isDefaultFg = VTERM_COLOR_IS_DEFAULT_FG(&cell.fg);
        const bool isDefaultBg = VTERM_COLOR_IS_DEFAULT_BG(&cell.bg);
        const QString fg = isDefaultFg ? QString() : vtermColorToHex(m_vts, cell.fg);
        const QString bg = isDefaultBg ? QString() : vtermColorToHex(m_vts, cell.bg);
        const bool bold = cell.attrs.bold;
        const bool italic = cell.attrs.italic;
        const bool underline = cell.attrs.underline != 0;

        // Start a new span if attributes changed
        if (fg != curFg || bg != curBg || bold != curBold ||
            italic != curItalic || underline != curUnderline) {
            closeSpan();
            curFg = fg;
            curBg = bg;
            curBold = bold;
            curItalic = italic;
            curUnderline = underline;

            // Only emit a span if we have non-default styling
            if (!curFg.isEmpty() || !curBg.isEmpty() || curBold || curItalic || curUnderline) {
                html.append(QStringLiteral("<span style=\""));
                if (!curFg.isEmpty())
                    html.append(QStringLiteral("color:%1;").arg(curFg));
                if (!curBg.isEmpty())
                    html.append(QStringLiteral("background-color:%1;").arg(curBg));
                if (curBold)
                    html.append(QStringLiteral("font-weight:bold;"));
                if (curItalic)
                    html.append(QStringLiteral("font-style:italic;"));
                if (curUnderline)
                    html.append(QStringLiteral("text-decoration:underline;"));
                html.append(QStringLiteral("\">"));
                spanOpen = true;
            }
        }

        appendCellText(html, cell);

        // Skip continuation cells for double-width characters
        if (cell.width > 1) {
            col += cell.width - 1;
        }
    }

    closeSpan();
    return html;
}
