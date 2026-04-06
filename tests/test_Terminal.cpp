#include <QtTest>
#include "backend/TerminalBackend.h"
#include "backend/TerminalModel.h"

class TerminalTest : public QObject {
    Q_OBJECT

private slots:
    void testBackendInitialState() {
        TerminalBackend backend;
        QCOMPARE(backend.rows(), 24);
        QCOMPARE(backend.cols(), 80);
        QCOMPARE(backend.cursorRow(), 0);
        QCOMPARE(backend.cursorCol(), 0);

        // Line should be all spaces initially
        const QString line = backend.getLineText(0);
        QCOMPARE(line.length(), 80);
        QCOMPARE(line.trimmed(), QString());
    }

    void testBackendResizing() {
        TerminalBackend backend;
        backend.resize(30, 100);
        QCOMPARE(backend.rows(), 30);
        QCOMPARE(backend.cols(), 100);

        const QString line = backend.getLineText(0);
        QCOMPARE(line.length(), 100);
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

    void testGetLineTextOutOfBounds() {
        TerminalBackend backend;
        QVERIFY(backend.getLineText(-1).isEmpty());
        QVERIFY(backend.getLineText(24).isEmpty());
        QVERIFY(backend.getLineText(999).isEmpty());
    }

    void testModelIntegration() {
        TerminalBackend backend;
        TerminalModel model;
        model.setBackend(&backend);

        QCOMPARE(model.rowCount(), 24);

        backend.resize(10, 80);
        QCOMPARE(model.rowCount(), 10);
    }

    void testModelFlatList() {
        TerminalModel model;
        // With parent index, should return 0 (flat list)
        QCOMPARE(model.rowCount(QModelIndex()), 0);
    }

    void testModelDataChanged() {
        TerminalBackend backend;
        TerminalModel model;
        model.setBackend(&backend);

        QSignalSpy spy(&model, &TerminalModel::dataChanged);
        emit backend.screenDamaged(0, 5);
        QCOMPARE(spy.count(), 1);
    }

    void testModelSetBackendDisconnects() {
        TerminalBackend backend1;
        TerminalBackend backend2;
        TerminalModel model;

        model.setBackend(&backend1);
        model.setBackend(&backend2);

        // Signals from old backend should not trigger updates
        QSignalSpy spy(&model, &TerminalModel::dataChanged);
        emit backend1.screenDamaged(0, 5);
        QCOMPARE(spy.count(), 0);

        emit backend2.screenDamaged(0, 5);
        QCOMPARE(spy.count(), 1);
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
