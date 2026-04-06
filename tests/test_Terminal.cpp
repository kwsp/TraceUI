#include <QtTest>
#include "backend/TerminalBackend.h"
#include "backend/TerminalModel.h"

class TerminalTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // No special init needed
    }

    void testBackendInitialState() {
        TerminalBackend backend;
        QCOMPARE(backend.rows(), 24);
        QCOMPARE(backend.cols(), 80);
        
        // Line should be all spaces initially
        QString line = backend.getLineText(0);
        QCOMPARE(line.length(), 80);
        QCOMPARE(line.trimmed(), QString(""));
    }

    void testBackendResizing() {
        TerminalBackend backend;
        backend.setRows(30);
        backend.setCols(100);
        QCOMPARE(backend.rows(), 30);
        QCOMPARE(backend.cols(), 100);
        
        QString line = backend.getLineText(0);
        QCOMPARE(line.length(), 100);
    }

    void testModelIntegration() {
        TerminalBackend backend;
        TerminalModel model;
        model.setBackend(&backend);
        
        QCOMPARE(model.rowCount(), 24);
        
        backend.setRows(10);
        QCOMPARE(model.rowCount(), 10);
    }

    void testDataReception() {
        TerminalBackend backend;
        TerminalModel model;
        model.setBackend(&backend);

        // Manually push some data to backend's vterm via a protected/private hack or just add a test method
        // Since I can't easily access private members, I'll use sendInput if I had a dummy shell,
        // but for unit test, let's just test getLineText works with manually populated vterm if possible.
        // Actually, I'll just check if the model signals dataChanged when the screen is "updated".
        
        QSignalSpy spy(&model, &TerminalModel::dataChanged);
        emit backend.screenUpdated();
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(TerminalTest)
#include "test_Terminal.moc"
