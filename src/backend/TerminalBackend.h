#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <vterm.h>

class TerminalBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(int rows READ rows WRITE setRows NOTIFY rowsChanged)
    Q_PROPERTY(int cols READ cols WRITE setCols NOTIFY colsChanged)

    Q_PROPERTY(int cursorRow READ cursorRow NOTIFY cursorMoved)
    Q_PROPERTY(int cursorCol READ cursorCol NOTIFY cursorMoved)

public:
    explicit TerminalBackend(QObject *parent = nullptr);
    ~TerminalBackend();

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }
    int cursorRow() const { return m_cursorRow; }
    int cursorCol() const { return m_cursorCol; }

    void setRows(int rows);
    void setCols(int cols);
    void setCursorPos(int row, int col);

    Q_INVOKABLE void sendInput(const QString &input);
    Q_INVOKABLE void start(const QString &shell = "/bin/zsh");

    QString getLineText(int row) const;

signals:
    void rowsChanged();
    void colsChanged();
    void cursorMoved();
    void dataReceived(const QByteArray &data);
    void screenUpdated();

private slots:
    void onPtyReadReady();

private:
    void setupVTerm();
    void resizePty();

    int m_rows = 24;
    int m_cols = 80;
    int m_cursorRow = 0;
    int m_cursorCol = 0;

    int m_masterFd = -1;
    pid_t m_childPid = -1;
    QSocketNotifier *m_notifier = nullptr;

    VTerm *m_vt = nullptr;
    VTermScreen *m_vts = nullptr;
};
