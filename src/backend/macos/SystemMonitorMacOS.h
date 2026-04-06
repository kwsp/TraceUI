#pragma once

#include "backend/SystemMonitor.h"
#include <QTimer>
#include <mach/mach.h>

class SystemMonitorMacOS : public SystemMonitor {
    Q_OBJECT

public:
    explicit SystemMonitorMacOS(QObject *parent = nullptr);
    SystemMonitorMacOS(const SystemMonitorMacOS &) = delete;
    SystemMonitorMacOS &operator=(const SystemMonitorMacOS &) = delete;
    SystemMonitorMacOS(SystemMonitorMacOS &&) = delete;
    SystemMonitorMacOS &operator=(SystemMonitorMacOS &&) = delete;
    ~SystemMonitorMacOS() override;

    void update() override;

private slots:
    void performUpdate();

private:
    void updateCpuUsage();
    void updateMemoryUsage();
    void updateLoadAverage();
    void updateTemperature();
    void updateUptime();

    QTimer m_timer;
    processor_info_array_t m_prevCpuInfo = nullptr;
    mach_msg_type_number_t m_numPrevCpuInfo = 0;
    unsigned int m_numCPUs = 0;
    time_t m_bootTimeSecs = 0;
};
