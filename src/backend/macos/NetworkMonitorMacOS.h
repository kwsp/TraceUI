#pragma once

#include "backend/NetworkMonitor.h"
#include <QTimer>
#include <QMap>

class NetworkMonitorMacOS : public NetworkMonitor {
    Q_OBJECT

public:
    explicit NetworkMonitorMacOS(QObject* parent = nullptr);
    ~NetworkMonitorMacOS() override = default;

    void update() override;

private slots:
    void performUpdate();

private:
    void updateNetworkTraffic();
    void updateActiveConnections();
    
    QTimer m_timer;
    
    struct TrafficData {
        uint64_t ibytes = 0;
        uint64_t obytes = 0;
        uint64_t timestampMs = 0;
    };
    
    QMap<QString, TrafficData> m_prevTrafficData;
};
