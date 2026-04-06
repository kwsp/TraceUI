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
[[nodiscard]] QString SystemMonitor::osType() const { return m_osType; }
[[nodiscard]] QString SystemMonitor::powerSource() const { return m_powerSource; }
[[nodiscard]] double SystemMonitor::cpuClockMin() const { return m_cpuClockMin; }
[[nodiscard]] double SystemMonitor::cpuClockMax() const { return m_cpuClockMax; }
[[nodiscard]] int SystemMonitor::totalTasks() const { return m_totalTasks; }
