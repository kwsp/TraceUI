#include "TerminalBackend.h"
#include <QtTest>

class TerminalTest : public QObject {
    Q_OBJECT

private slots:
    void testBackendInitialState() {
        TerminalBackend backend;
        QCOMPARE(backend.rows(), 24);
        QCOMPARE(backend.cols(), 80);
        QCOMPARE(backend.cursorRow(), 0);
        QCOMPARE(backend.cursorCol(), 0);
    }

    void testBackendResizing() {
        TerminalBackend backend;
        backend.resize(30, 100);
        QCOMPARE(backend.rows(), 30);
        QCOMPARE(backend.cols(), 100);
    }

    void testResizeRejectsInvalidValues() {
        TerminalBackend backend;
        backend.resize(0, 0);
        QCOMPARE(backend.rows(), 24);
        QCOMPARE(backend.cols(), 80);

        backend.resize(-1, 50);
        QCOMPARE(backend.rows(), 24);

        backend.resize(10, -5);
        QCOMPARE(backend.cols(), 80);
    }

    void testCursorPosition() {
        TerminalBackend backend;
        QSignalSpy spy(&backend, &TerminalBackend::cursorMoved);

        backend.setCursorPos(5, 10);
        QCOMPARE(backend.cursorRow(), 5);
        QCOMPARE(backend.cursorCol(), 10);
        QCOMPARE(spy.count(), 1);

        // Same position should not re-emit
        backend.setCursorPos(5, 10);
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(TerminalTest)
#include "test_Terminal.moc"
