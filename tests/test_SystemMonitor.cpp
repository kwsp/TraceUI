#include <QtGlobal>
#ifndef Q_OS_MACOS
int main(int, char*[]) { return 0; }
#else
#include <QtTest>
#include <QCoreApplication>
#include "backend/macos/SystemMonitorMacOS.h"

class SystemMonitorTest : public QObject {
    Q_OBJECT

private slots:
    void testInitialValues() {
        SystemMonitorMacOS monitor;
        
        QVERIFY(monitor.ramTotalMB() == 0);
        QVERIFY(monitor.ramUsedMB() == 0);
        QVERIFY(monitor.cpuUsagePercent().isEmpty());
    }

    void testUpdateProvidesData() {
        SystemMonitorMacOS monitor;
        monitor.update();
        
        QVERIFY(monitor.ramTotalMB() > 0);
        
        monitor.update();
        QVERIFY(!monitor.cpuUsagePercent().isEmpty());
        QVERIFY(monitor.ramUsedMB() > 0);
        QVERIFY(monitor.ramUsedMB() <= monitor.ramTotalMB());
        QVERIFY(monitor.loadAverage1m() >= 0.0);
    }
};

QTEST_MAIN(SystemMonitorTest)
#include "test_SystemMonitor.moc"
#endif
