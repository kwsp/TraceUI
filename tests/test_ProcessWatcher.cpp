#include <QtGlobal>
#ifndef Q_OS_MACOS
int main(int, char*[]) { return 0; }
#else
#include <QtTest>
#include <QCoreApplication>
#include "backend/macos/ProcessWatcherMacOS.h"

class ProcessWatcherTest : public QObject {
    Q_OBJECT

private slots:
    void testInitialValues() {
        ProcessWatcherMacOS watcher;
        QVERIFY(watcher.processes().isEmpty());
    }

    void testUpdateProvidesProcesses() {
        ProcessWatcherMacOS watcher;
        watcher.update();
        
        QVERIFY(!watcher.processes().isEmpty());
        
        bool foundTraceUI = false;
        const auto procs = watcher.processes();
        for (const QVariant& pVar : procs) {
            QVariantMap pInfo = pVar.toMap();
            QVERIFY(pInfo.contains("pid"));
            QVERIFY(pInfo.contains("name"));
            QVERIFY(pInfo.contains("ramMB"));
            if (pInfo["name"].toString() == "test_ProcessWatcher" || pInfo["name"].toString() == "launchd") {
                foundTraceUI = true;
            }
        }
        QVERIFY(foundTraceUI);
    }
};

QTEST_MAIN(ProcessWatcherTest)
#include "test_ProcessWatcher.moc"
#endif
