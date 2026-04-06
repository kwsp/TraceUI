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

static VTermScreenCallbacks screen_callbacks = {
    .damage = screen_damage,
    .movecursor = screen_movecursor,
};

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
    m_vt = vterm_new(m_rows, m_cols);
    vterm_set_utf8(m_vt, 1);
    m_vts = vterm_obtain_screen(m_vt);
    vterm_screen_set_callbacks(m_vts, &screen_callbacks, this);
    vterm_screen_reset(m_vts, 1);
}

void TerminalBackend::start(const QString &shell) {
    struct winsize ws = { (unsigned short)m_rows, (unsigned short)m_cols, 0, 0 };
    
    m_childPid = forkpty(&m_masterFd, nullptr, nullptr, &ws);
    
    if (m_childPid == 0) { // Child process
        setenv("TERM", "xterm-256color", 1);
        const char *shellPath = shell.toLocal8Bit().constData();
        execl(shellPath, shellPath, nullptr);
        _exit(1);
    } else if (m_childPid > 0) { // Parent process
        fcntl(m_masterFd, F_SETFL, O_NONBLOCK);
        m_notifier = new QSocketNotifier(m_masterFd, QSocketNotifier::Read, this);
        connect(m_notifier, &QSocketNotifier::activated, this, &TerminalBackend::onPtyReadReady);
    } else {
        qWarning() << "forkpty failed";
    }
}

void TerminalBackend::onPtyReadReady() {
    char buffer[4096];
    ssize_t bytesRead = read(m_masterFd, buffer, sizeof(buffer));
    if (bytesRead > 0) {
        vterm_input_write(m_vt, buffer, bytesRead);
        emit dataReceived(QByteArray(buffer, bytesRead));
    } else if (bytesRead == 0 || (bytesRead < 0 && errno != EAGAIN)) {
        m_notifier->setEnabled(false);
        qDebug() << "PTY closed or error";
    }
}

void TerminalBackend::sendInput(const QString &input) {
    if (m_masterFd != -1) {
        QByteArray data = input.toLocal8Bit();
        write(m_masterFd, data.data(), data.size());
    }
}

void TerminalBackend::setRows(int rows) {
    if (m_rows != rows) {
        m_rows = rows;
        vterm_set_size(m_vt, m_rows, m_cols);
        resizePty();
        emit rowsChanged();
    }
}

void TerminalBackend::setCols(int cols) {
    if (m_cols != cols) {
        m_cols = cols;
        vterm_set_size(m_vt, m_rows, m_cols);
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
    if (row < 0 || row >= m_rows) return QString();
    
    QString line;
    for (int col = 0; col < m_cols; ++col) {
        VTermScreenCell cell;
        VTermPos pos = { row, col };
        vterm_screen_get_cell(m_vts, pos, &cell);
        
        if (cell.chars[0] == 0) {
            line.append(" ");
        } else {
            // libvterm stores chars as uint32_t (Unicode points)
            for (int i = 0; i < VTERM_MAX_CHARS_PER_CELL && cell.chars[i]; ++i) {
                line.append(QChar(cell.chars[i]));
            }
        }
    }
    return line;
}
