#include "SystemMonitor.h"

SystemMonitor::SystemMonitor(QObject* parent) : QObject(parent) {}

[[nodiscard]] QList<double> SystemMonitor::cpuUsagePercent() const { return m_cpuUsagePercent; }
[[nodiscard]] int SystemMonitor::ramUsedMB() const { return m_ramUsedMB; }
[[nodiscard]] int SystemMonitor::ramTotalMB() const { return m_ramTotalMB; }
[[nodiscard]] int SystemMonitor::swapUsedMB() const { return m_swapUsedMB; }
[[nodiscard]] double SystemMonitor::cpuTempCelsius() const { return m_cpuTempCelsius; }
[[nodiscard]] double SystemMonitor::loadAverage1m() const { return m_loadAverage1m; }
[[nodiscard]] double SystemMonitor::cpuUsageTotal() const { return m_cpuUsageTotal; }
[[nodiscard]] double SystemMonitor::cpuUsageUser() const { return m_cpuUsageUser; }
[[nodiscard]] double SystemMonitor::cpuUsageSystem() const { return m_cpuUsageSystem; }
[[nodiscard]] QString SystemMonitor::cpuName() const { return m_cpuName; }
[[nodiscard]] QString SystemMonitor::uptime() const { return m_uptime; }
