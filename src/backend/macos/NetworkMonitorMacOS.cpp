#include "NetworkMonitorMacOS.h"
#include <QDateTime>
#include <QRegularExpression>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

static constexpr int kPollIntervalMs = 2000;
static constexpr int kPingWindowSize = 20; // sliding window for packet loss

NetworkMonitorMacOS::NetworkMonitorMacOS(QObject *parent) : NetworkMonitor(parent) {
    m_pingProcess = new QProcess(this);
    connect(m_pingProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &NetworkMonitorMacOS::onPingFinished);

    connect(&m_timer, &QTimer::timeout, this, &NetworkMonitorMacOS::performUpdate);
    m_timer.start(kPollIntervalMs);
}

void NetworkMonitorMacOS::update() {
    performUpdate();
}

void NetworkMonitorMacOS::performUpdate() {
    updateNetworkTraffic();
    updateActiveConnections();
    updateIPv4Address();
    startPing();
    emit dataUpdated();
}

// ── IPv4 address from primary interface ──────────────────────────────────────

void NetworkMonitorMacOS::updateIPv4Address() {
    struct ifaddrs *ifap = nullptr;
    if (getifaddrs(&ifap) != 0)
        return;

    m_ipv4Address = "0.0.0.0";
    m_isOnline = false;

    for (struct ifaddrs *ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        QString name(ifa->ifa_name);
        // Skip loopback and virtual interfaces
        if (name == "lo0" || name.startsWith("utun") || name.startsWith("awdl") ||
            name.startsWith("llw") || name.startsWith("bridge") || name.startsWith("gif") ||
            name.startsWith("stf") || name.startsWith("anpi"))
            continue;

        auto *sin = reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr);
        char buf[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf))) {
            QString addr = QString::fromUtf8(buf);
            if (addr != "0.0.0.0" && addr != "127.0.0.1") {
                m_ipv4Address = addr;
                m_interface = name;
                m_isOnline = true;
                break;
            }
        }
    }
    freeifaddrs(ifap);
}

// ── Ping (async, non-blocking) ───────────────────────────────────────────────

void NetworkMonitorMacOS::startPing() {
    if (m_pingInFlight || !m_isOnline)
        return;

    m_pingInFlight = true;
    // Single ping with 2 second timeout (-t on macOS, -W is waittime in ms)
    m_pingProcess->start("/sbin/ping", {"-c", "1", "-t", "2", "1.1.1.1"});
}

void NetworkMonitorMacOS::onPingFinished(int exitCode, QProcess::ExitStatus /*status*/) {
    m_pingInFlight = false;

    // Sliding window for packet loss
    m_pingsSent++;
    if (m_pingsSent > kPingWindowSize) {
        m_pingsSent /= 2;
        m_pingsLost /= 2;
    }

    QString output = m_pingProcess->readAllStandardOutput();
    QString errOutput = m_pingProcess->readAllStandardError();

    if (exitCode != 0) {
        qDebug() << "[ping] exit code:" << exitCode
                 << "stdout:" << output.left(200)
                 << "stderr:" << errOutput.left(200);
        m_pingsLost++;
        m_pingMs = -1;
    } else {
        // Parse round-trip time from ping output
        // Example line: "64 bytes from 1.1.1.1: icmp_seq=0 ttl=55 time=4.123 ms"
        static QRegularExpression re(R"(time[=<](\d+\.?\d*)\s*ms)");
        QRegularExpressionMatch match = re.match(output);
        if (match.hasMatch()) {
            m_pingMs = qRound(match.captured(1).toDouble());
        } else {
            m_pingMs = -1;
        }
    }

    m_packetLossPct =
        (m_pingsSent > 0) ? static_cast<double>(m_pingsLost) / m_pingsSent * 100.0 : 0.0;

    emit dataUpdated();
}

// ── Network traffic ──────────────────────────────────────────────────────────

void NetworkMonitorMacOS::updateNetworkTraffic() {
    struct ifaddrs *ifap, *ifa;
    if (getifaddrs(&ifap) != 0)
        return;

    uint64_t currentTotalIBytes = 0;
    uint64_t currentTotalOBytes = 0;
    bool isVpnActive = false;

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr || ifa->ifa_addr->sa_family != AF_LINK)
            continue;

        QString ifaceName(ifa->ifa_name);
        if (ifaceName.startsWith("utun") || ifaceName.startsWith("wg"))
            isVpnActive = true;

        if (ifaceName == m_interface || isVpnActive) {
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

// ── Active connections ───────────────────────────────────────────────────────

void NetworkMonitorMacOS::updateActiveConnections() {
    // net.inet.tcp.pcbcount gives the system-wide TCP PCB (protocol control block) count.
    // This is the number of active TCP sockets, equivalent to what netstat reports.
    int tcpCount = 0;
    size_t len = sizeof(tcpCount);
    if (sysctlbyname("net.inet.tcp.pcbcount", &tcpCount, &len, nullptr, 0) == 0) {
        m_activeConnections = tcpCount;
    }
}
