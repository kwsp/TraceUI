#include "NetworkMonitor.h"

NetworkMonitor::NetworkMonitor(QObject *parent) : QObject(parent) {}

double NetworkMonitor::downloadBytesPerSec() const {
    return m_downloadBytesPerSec;
}
double NetworkMonitor::uploadBytesPerSec() const {
    return m_uploadBytesPerSec;
}
int NetworkMonitor::activeConnections() const {
    return m_activeConnections;
}
QString NetworkMonitor::mainInterface() const {
    return m_interface;
}
bool NetworkMonitor::vpnActive() const {
    return m_vpnActive;
}
QString NetworkMonitor::ipv4Address() const {
    return m_ipv4Address;
}
bool NetworkMonitor::isOnline() const {
    return m_isOnline;
}
int NetworkMonitor::pingMs() const {
    return m_pingMs;
}
double NetworkMonitor::packetLossPct() const {
    return m_packetLossPct;
}
