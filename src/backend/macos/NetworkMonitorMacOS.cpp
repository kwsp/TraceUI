#include "NetworkMonitorMacOS.h"
#include <QDateTime>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_mib.h>
#include <net/route.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

NetworkMonitorMacOS::NetworkMonitorMacOS(QObject *parent) : NetworkMonitor(parent) {
    connect(&m_timer, &QTimer::timeout, this, &NetworkMonitorMacOS::performUpdate);
    m_timer.start(2000); // 2 seconds polling
}

void NetworkMonitorMacOS::update() {
    performUpdate();
}

void NetworkMonitorMacOS::performUpdate() {
    updateNetworkTraffic();
    updateActiveConnections();
    m_ipv4Address = "192.168.1.42"; // Placeholder
    m_isOnline = true;              // Placeholder
    m_pingMs = 15;                  // Placeholder
    m_packetLossPct = 0.05;         // Placeholder
    emit dataUpdated();
}

void NetworkMonitorMacOS::updateNetworkTraffic() {
    struct ifaddrs *ifap, *ifa;
    if (getifaddrs(&ifap) != 0) {
        return;
    }

    uint64_t currentTotalIBytes = 0;
    uint64_t currentTotalOBytes = 0;
    bool isVpnActive = false;

    // A simplified traffic counting summing over typical main interfaces (e.g. en0, utun*)
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr || ifa->ifa_addr->sa_family != AF_LINK) {
            continue;
        }

        QString ifaceName(ifa->ifa_name);
        if (ifaceName.startsWith("utun") || ifaceName.startsWith("wg")) {
            isVpnActive = true;
        }

        if (ifaceName == m_interface || isVpnActive) { // track main or vpn interface
            struct if_data *ifd = (struct if_data *)ifa->ifa_data;
            currentTotalIBytes += ifd->ifi_ibytes;
            currentTotalOBytes += ifd->ifi_obytes;
        }
    }
    freeifaddrs(ifap);

    uint64_t currentMs = QDateTime::currentMSecsSinceEpoch();

    if (!m_prevTrafficData.isEmpty() && m_prevTrafficData.contains("total")) {
        TrafficData prev = m_prevTrafficData["total"];
        double timeDiffSec = (currentMs - prev.timestampMs) / 1000.0;
        if (timeDiffSec > 0.0) {
            m_downloadBytesPerSec = (currentTotalIBytes >= prev.ibytes)
                                        ? (currentTotalIBytes - prev.ibytes) / timeDiffSec
                                        : 0.0;
            m_uploadBytesPerSec = (currentTotalOBytes >= prev.obytes)
                                      ? (currentTotalOBytes - prev.obytes) / timeDiffSec
                                      : 0.0;
        }
    }

    TrafficData currentData;
    currentData.ibytes = currentTotalIBytes;
    currentData.obytes = currentTotalOBytes;
    currentData.timestampMs = currentMs;
    m_prevTrafficData["total"] = currentData;
    m_vpnActive = isVpnActive;
}

void NetworkMonitorMacOS::updateActiveConnections() {
    // Stubbed. In full implementation, parsing `lsof` or `netstat` or using libproc
    // `proc_pidfdinfo` for sockets is needed Setting a fake but non-zero number for architecture
    // testing
    m_activeConnections = 100;
}
