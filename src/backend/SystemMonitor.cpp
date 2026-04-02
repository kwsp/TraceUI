#include "SystemMonitor.h"

SystemMonitor::SystemMonitor(QObject* parent) : QObject(parent) {}

[[nodiscard]] QList<double> SystemMonitor::cpuUsagePercent() const { return m_cpuUsagePercent; }
[[nodiscard]] int SystemMonitor::ramUsedMB() const { return m_ramUsedMB; }
[[nodiscard]] int SystemMonitor::ramTotalMB() const { return m_ramTotalMB; }
[[nodiscard]] int SystemMonitor::swapUsedMB() const { return m_swapUsedMB; }
[[nodiscard]] double SystemMonitor::cpuTempCelsius() const { return m_cpuTempCelsius; }
[[nodiscard]] double SystemMonitor::loadAverage1m() const { return m_loadAverage1m; }
