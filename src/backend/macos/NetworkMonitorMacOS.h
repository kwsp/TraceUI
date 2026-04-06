#pragma once

#include "backend/NetworkMonitor.h"
#include <QMap>
#include <QProcess>
#include <QTimer>

class NetworkMonitorMacOS : public NetworkMonitor {
    Q_OBJECT

public:
    explicit NetworkMonitorMacOS(QObject *parent = nullptr);
    NetworkMonitorMacOS(const NetworkMonitorMacOS &) = delete;
    NetworkMonitorMacOS &operator=(const NetworkMonitorMacOS &) = delete;
    NetworkMonitorMacOS(NetworkMonitorMacOS &&) = delete;
    NetworkMonitorMacOS &operator=(NetworkMonitorMacOS &&) = delete;
    ~NetworkMonitorMacOS() override = default;

    void update() override;

private slots:
    void performUpdate();
    void onPingFinished(int exitCode, QProcess::ExitStatus status);

private:
    void updateNetworkTraffic();
    void updateActiveConnections();
    void updateIPv4Address();
    void startPing();

    QTimer m_timer;
    QProcess *m_pingProcess = nullptr;
    bool m_pingInFlight = false;

    // Track cumulative packet loss over a sliding window
    int m_pingsSent = 0;
    int m_pingsLost = 0;

    struct TrafficData {
        uint64_t ibytes{};
        uint64_t obytes{};
        uint64_t timestampMs{};
    };

    QMap<QString, TrafficData> m_prevTrafficData;
};
