#include "NetworkMonitor.h"

NetworkMonitor::NetworkMonitor(QObject* parent) : QObject(parent) {}

double NetworkMonitor::downloadBytesPerSec() const { return m_downloadBytesPerSec; }
double NetworkMonitor::uploadBytesPerSec() const { return m_uploadBytesPerSec; }
int NetworkMonitor::activeConnections() const { return m_activeConnections; }
QString NetworkMonitor::interface() const { return m_interface; }
bool NetworkMonitor::vpnActive() const { return m_vpnActive; }
