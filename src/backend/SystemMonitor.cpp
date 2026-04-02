#include "SystemMonitor.h"

SystemMonitor::SystemMonitor(QObject* parent) : QObject(parent) {}

QVariantList SystemMonitor::cpuUsagePercent() const { return m_cpuUsagePercent; }
int SystemMonitor::ramUsedMB() const { return m_ramUsedMB; }
int SystemMonitor::ramTotalMB() const { return m_ramTotalMB; }
int SystemMonitor::swapUsedMB() const { return m_swapUsedMB; }
double SystemMonitor::cpuTempCelsius() const { return m_cpuTempCelsius; }
double SystemMonitor::loadAverage1m() const { return m_loadAverage1m; }
