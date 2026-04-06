#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <QTimer>

#include <memory>
#include <vterm.h>

/// RAII wrapper for POSIX file descriptors.
class FileDescriptor {
public:
    FileDescriptor() = default;
    explicit FileDescriptor(int fd) : m_fd(fd) {}
    ~FileDescriptor();

    FileDescriptor(const FileDescriptor &) = delete;
    FileDescriptor &operator=(const FileDescriptor &) = delete;
    FileDescriptor(FileDescriptor &&other) noexcept;
    FileDescriptor &operator=(FileDescriptor &&other) noexcept;

    [[nodiscard]] int get() const { return m_fd; }
    [[nodiscard]] bool isValid() const { return m_fd >= 0; }
    int release();
    void reset(int fd = -1);

private:
    int m_fd = -1;
};

/// Custom deleter for VTerm, enabling use with std::unique_ptr.
struct VTermDeleter {
    void operator()(VTerm *vt) const {
        if (vt) vterm_free(vt);
    }
};

using VTermPtr = std::unique_ptr<VTerm, VTermDeleter>;

class TerminalBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(int rows READ rows WRITE setRows NOTIFY rowsChanged)
    Q_PROPERTY(int cols READ cols WRITE setCols NOTIFY colsChanged)
    Q_PROPERTY(int cursorRow READ cursorRow NOTIFY cursorMoved)
    Q_PROPERTY(int cursorCol READ cursorCol NOTIFY cursorMoved)

public:
    explicit TerminalBackend(QObject *parent = nullptr);
    ~TerminalBackend() override;

    // Non-copyable, non-movable (QObject + OS resources)
    TerminalBackend(const TerminalBackend &) = delete;
    TerminalBackend &operator=(const TerminalBackend &) = delete;

    [[nodiscard]] int rows() const { return m_rows; }
    [[nodiscard]] int cols() const { return m_cols; }
    [[nodiscard]] int cursorRow() const { return m_cursorRow; }
    [[nodiscard]] int cursorCol() const { return m_cursorCol; }

    void setRows(int rows);
    void setCols(int cols);
    Q_INVOKABLE void resize(int rows, int cols);
    void setCursorPos(int row, int col);
    void writeToPty(const QByteArray &data);

    Q_INVOKABLE void sendInput(const QString &input);
    Q_INVOKABLE void start(const QString &shell = QString());

    [[nodiscard]] QString getLineText(int row) const;
    [[nodiscard]] QString getLineHtml(int row) const;

signals:
    void rowsChanged();
    void colsChanged();
    void cursorMoved();
    void screenUpdated();

private slots:
    void onPtyReadReady();

private:
    void setupVTerm();
    void resizePty();
    void cleanupChild();

    int m_rows = 24;
    int m_cols = 80;
    int m_cursorRow = 0;
    int m_cursorCol = 0;

    FileDescriptor m_masterFd;
    pid_t m_childPid = -1;
    QSocketNotifier *m_notifier = nullptr;

    VTermPtr m_vt;
    VTermScreen *m_vts = nullptr;  // Owned by m_vt, not freed separately
};
