#include <QtGlobal>
#ifndef Q_OS_MACOS
int main(int, char *[]) {
    return 0;
}
#else
#include "backend/macos/NetworkMonitorMacOS.h"
#include <QCoreApplication>
#include <QThread>
#include <QtTest>

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
        QThread::msleep(100);
        monitor.update();

        QVERIFY(monitor.downloadBytesPerSec() >= 0.0);
        QVERIFY(monitor.uploadBytesPerSec() >= 0.0);
        QVERIFY(monitor.activeConnections() >= 0);
    }
};

QTEST_MAIN(NetworkMonitorTest)
#include "test_NetworkMonitor.moc"
#endif
