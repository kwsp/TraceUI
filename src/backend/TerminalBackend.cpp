#include "TerminalBackend.h"
#include <QDebug>
#include <QThread>

#ifdef Q_OS_MACOS
#include <util.h>
#elif defined(Q_OS_LINUX)
#include <pty.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

static int screen_damage(VTermRect rect, void *user) {
    auto *backend = static_cast<TerminalBackend *>(user);
    emit backend->screenUpdated();
    return 1;
}

static int screen_movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user) {
    auto *backend = static_cast<TerminalBackend *>(user);
    backend->setCursorPos(pos.row, pos.col);
    return 1;
}

static int screen_settermprop(VTermProp prop, VTermValue *val, void *user) {
    return 1;
}

static VTermScreenCallbacks screen_callbacks = {
    .damage = screen_damage,
    .moverect = nullptr,
    .movecursor = screen_movecursor,
    .settermprop = screen_settermprop,
    .bell = nullptr,
    .resize = nullptr,
    .sb_pushline = nullptr,
    .sb_popline = nullptr,
};

static void on_vterm_output(const char *s, size_t len, void *user) {
    auto *backend = static_cast<TerminalBackend *>(user);
    backend->writeToPty(QByteArray(s, len));
}

TerminalBackend::TerminalBackend(QObject *parent)
    : QObject(parent)
{
    setupVTerm();
}

TerminalBackend::~TerminalBackend() {
    if (m_vt) vterm_free(m_vt);
    if (m_masterFd != -1) close(m_masterFd);
}

void TerminalBackend::setupVTerm() {
    if (m_vt) vterm_free(m_vt);
    m_vt = vterm_new(m_rows, m_cols);
    vterm_set_utf8(m_vt, 1);
    vterm_output_set_callback(m_vt, on_vterm_output, this);
    m_vts = vterm_obtain_screen(m_vt);
    vterm_screen_set_callbacks(m_vts, &screen_callbacks, this);
    vterm_screen_reset(m_vts, 1);
}

void TerminalBackend::start(const QString &shell) {
    qDebug() << "Starting shell:" << shell << "rows:" << m_rows << "cols:" << m_cols;
    
    // Initial test message to verify rendering
    const char *msg = "TraceUI Terminal Initializing...\r\n";
    vterm_input_write(m_vt, msg, strlen(msg));

    struct winsize ws = { (unsigned short)m_rows, (unsigned short)m_cols, 0, 0 };
    
    m_childPid = forkpty(&m_masterFd, nullptr, nullptr, &ws);
    
    if (m_childPid == 0) { // Child process
        setenv("TERM", "xterm-256color", 1);
        const char *shellPath = shell.toLocal8Bit().constData();
        execl(shellPath, shellPath, nullptr);
        _exit(1);
    } else if (m_childPid > 0) { // Parent process
        qDebug() << "Parent process: child PID is" << m_childPid << "master FD is" << m_masterFd;
        fcntl(m_masterFd, F_SETFL, O_NONBLOCK);
        m_notifier = new QSocketNotifier(m_masterFd, QSocketNotifier::Read, this);
        connect(m_notifier, &QSocketNotifier::activated, this, &TerminalBackend::onPtyReadReady);
    } else {
        qWarning() << "forkpty failed:" << strerror(errno);
    }
}

void TerminalBackend::onPtyReadReady() {
    char buffer[4096];
    ssize_t bytesRead = read(m_masterFd, buffer, sizeof(buffer));
    if (bytesRead > 0) {
        qDebug() << "PTY -> vterm:" << bytesRead << "bytes:" << QByteArray(buffer, bytesRead).toHex(':');
        vterm_input_write(m_vt, buffer, bytesRead);
    } else if (bytesRead == 0 || (bytesRead < 0 && errno != EAGAIN)) {
        m_notifier->setEnabled(false);
        qDebug() << "PTY closed or error";
    }
}

void TerminalBackend::sendInput(const QString &input) {
    if (m_masterFd != -1) {
        QByteArray data = input.toUtf8();
        write(m_masterFd, data.data(), data.size());
    }
}

void TerminalBackend::writeToPty(const QByteArray &data) {
    if (m_masterFd != -1) {
        write(m_masterFd, data.data(), data.size());
    }
}

void TerminalBackend::setRows(int rows) {
    if (rows <= 0) return;
    if (m_rows != rows) {
        m_rows = rows;
        if (m_vt) vterm_set_size(m_vt, m_rows, m_cols);
        resizePty();
        emit rowsChanged();
    }
}

void TerminalBackend::setCols(int cols) {
    if (cols <= 0) return;
    if (m_cols != cols) {
        m_cols = cols;
        if (m_vt) vterm_set_size(m_vt, m_rows, m_cols);
        resizePty();
        emit colsChanged();
    }
}

void TerminalBackend::resizePty() {
    if (m_masterFd != -1) {
        struct winsize ws = { (unsigned short)m_rows, (unsigned short)m_cols, 0, 0 };
        ioctl(m_masterFd, TIOCSWINSZ, &ws);
    }
}

void TerminalBackend::setCursorPos(int row, int col) {
    if (m_cursorRow != row || m_cursorCol != col) {
        m_cursorRow = row;
        m_cursorCol = col;
        emit cursorMoved();
    }
}

QString TerminalBackend::getLineText(int row) const {
    vterm_screen_flush_damage(m_vts);
    if (row < 0 || row >= m_rows) return QString();
    
    QString line;
    for (int col = 0; col < m_cols; ++col) {
        VTermScreenCell cell;
        VTermPos pos = { row, col };
        vterm_screen_get_cell(m_vts, pos, &cell);
        
        if (cell.chars[0] == 0) {
            line.append(" ");
        } else {
            for (int i = 0; i < VTERM_MAX_CHARS_PER_CELL && cell.chars[i]; ++i) {
                uint32_t c = cell.chars[i];
                if (QChar::requiresSurrogates(c)) {
                    line.append(QChar::highSurrogate(c));
                    line.append(QChar::lowSurrogate(c));
                } else {
                    line.append(QChar(static_cast<ushort>(c)));
                }
            }
        }
    }
    // Trim trailing spaces for better performance/rendering, but only if we want.
    // For a terminal, trailing spaces are often part of the screen state.
    return line;
}
