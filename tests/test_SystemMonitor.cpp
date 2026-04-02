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
        
        // Sometimes CPU requires a second poll to get the diff.
        monitor.update();
        QVERIFY(!monitor.cpuUsagePercent().isEmpty());
        QVERIFY(monitor.ramUsedMB() > 0);
        QVERIFY(monitor.ramUsedMB() <= monitor.ramTotalMB());
        
        // Verify load average is positive or 0
        QVERIFY(monitor.loadAverage1m() >= 0.0);
    }
};

QTEST_MAIN(SystemMonitorTest)

#include "test_SystemMonitor.moc"
