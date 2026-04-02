#include <QtTest>
#include <QCoreApplication>
#include <QThread>
#include "backend/macos/NetworkMonitorMacOS.h"

class NetworkMonitorTest : public QObject {
    Q_OBJECT

private slots:
    void testInitialValues() {
        NetworkMonitorMacOS monitor;
        
        QVERIFY(monitor.downloadBytesPerSec() == 0.0);
        QVERIFY(monitor.uploadBytesPerSec() == 0.0);
    }

    void testUpdateProvidesTraffic() {
        NetworkMonitorMacOS monitor;
        monitor.update();
        
        // Let time pass a bit for speed diff
        QThread::msleep(100);
        
        monitor.update(); // second update computes speeds
        
        QVERIFY(monitor.downloadBytesPerSec() >= 0.0);
        QVERIFY(monitor.uploadBytesPerSec() >= 0.0);
        QVERIFY(monitor.activeConnections() >= 0);
    }
};

QTEST_MAIN(NetworkMonitorTest)

#include "test_NetworkMonitor.moc"
