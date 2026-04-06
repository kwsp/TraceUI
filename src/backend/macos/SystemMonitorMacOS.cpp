#include "SystemMonitorMacOS.h"
#include <IOKit/IOKitLib.h>
#include <IOKit/ps/IOPSKeys.h>
#include <IOKit/ps/IOPowerSources.h>
#include <cstdlib>
#include <cstring>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#include <sys/time.h>

// ── SMC temperature reading ──────────────────────────────────────────────────
// macOS exposes CPU temperature via the AppleSMC kernel extension.
// The SMC user-client expects a specific 104-byte struct layout.

namespace {

// The SMC user-client struct must be exactly 80 bytes.
// Layout from osx-cpu-temp / smcFanControl open-source projects.
struct SMCVersion {
    unsigned char major;
    unsigned char minor;
    unsigned char build;
    unsigned char reserved;
    unsigned short release;
};

struct SMCPLimitData {
    uint16_t version;
    uint16_t length;
    uint32_t cpuPLimit;
    uint32_t gpuPLimit;
    uint32_t memPLimit;
};

struct SMCKeyInfoData {
    uint32_t dataSize;
    uint32_t dataType;
    uint8_t dataAttributes;
};

struct SMCKeyData {
    uint32_t key;
    SMCVersion vers;
    SMCPLimitData pLimitData;
    SMCKeyInfoData keyInfo;
    uint8_t result;
    uint8_t status;
    uint8_t data8;
    uint32_t data32;
    uint8_t bytes[32];
};

static_assert(sizeof(SMCKeyData) == 80, "SMCKeyData must be 80 bytes");

static constexpr uint8_t kSMCGetKeyInfo = 9;
static constexpr uint8_t kSMCReadKey = 5;

static uint32_t fourCC(const char c[4]) {
    return (uint32_t(c[0]) << 24) | (uint32_t(c[1]) << 16) | (uint32_t(c[2]) << 8) | uint32_t(c[3]);
}

static io_connect_t openSMC() {
    io_service_t svc =
        IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("AppleSMC"));
    if (svc == IO_OBJECT_NULL)
        return IO_OBJECT_NULL;
    io_connect_t conn = IO_OBJECT_NULL;
    IOServiceOpen(svc, mach_task_self(), 0, &conn);
    IOObjectRelease(svc);
    return conn;
}

// FourCC constants for SMC data types
static constexpr uint32_t kSMCTypeSP78 = ('s' << 24) | ('p' << 16) | ('7' << 8) | '8';
static constexpr uint32_t kSMCTypeFLT  = ('f' << 24) | ('l' << 16) | ('t' << 8) | ' ';

static double readSMCTemp(io_connect_t conn, const char key[4]) {
    if (conn == IO_OBJECT_NULL)
        return -1.0;

    SMCKeyData in{};
    SMCKeyData out{};

    // Step 1: Get key info (data type & size)
    in.key = fourCC(key);
    in.data8 = kSMCGetKeyInfo;
    size_t outSize = sizeof(SMCKeyData);
    if (IOConnectCallStructMethod(conn, 2, &in, sizeof(in), &out, &outSize) != KERN_SUCCESS)
        return -1.0;

    if (out.keyInfo.dataSize == 0)
        return -1.0; // key doesn't exist on this hardware

    // Step 2: Read the value using the key info from step 1
    in.keyInfo = out.keyInfo;
    in.data8 = kSMCReadKey;
    outSize = sizeof(SMCKeyData);
    if (IOConnectCallStructMethod(conn, 2, &in, sizeof(in), &out, &outSize) != KERN_SUCCESS)
        return -1.0;

    // Decode based on data type
    uint32_t dataType = out.keyInfo.dataType;
    if (dataType == kSMCTypeFLT && out.keyInfo.dataSize == 4) {
        // IEEE 754 float (Apple Silicon)
        float val;
        memcpy(&val, out.bytes, sizeof(float));
        return static_cast<double>(val);
    }
    if (dataType == kSMCTypeSP78 && out.keyInfo.dataSize >= 2) {
        // Signed 7.8 fixed-point (Intel Macs)
        auto raw = static_cast<int16_t>((out.bytes[0] << 8) | out.bytes[1]);
        return raw / 256.0;
    }
    return -1.0; // unknown type
}

} // namespace

// ── CPU frequency from sysctl ────────────────────────────────────────────────

static double getCpuFreqGHz(const char *name) {
    uint64_t freq = 0;
    size_t len = sizeof(freq);
    if (sysctlbyname(name, &freq, &len, nullptr, 0) == 0 && freq > 0) {
        return static_cast<double>(freq) / 1e9;
    }
    return 0.0;
}

// ── Constructor ──────────────────────────────────────────────────────────────

SystemMonitorMacOS::SystemMonitorMacOS(QObject *parent) : SystemMonitor(parent) {
    int mib[2U] = {CTL_HW, HW_NCPU};
    size_t sizeOfNumCPUs = sizeof(m_numCPUs);
    sysctl(mib, 2U, &m_numCPUs, &sizeOfNumCPUs, NULL, 0);

    char buffer[256];
    size_t bufferlen = sizeof(buffer);
    if (sysctlbyname("machdep.cpu.brand_string", &buffer, &bufferlen, NULL, 0) == 0) {
        m_cpuName = QString::fromUtf8(buffer);
    } else {
        m_cpuName = "Unknown CPU";
    }

    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    uint64_t memSizeBytes = 0;
    size_t memSizeBytesLen = sizeof(memSizeBytes);
    sysctl(mib, 2U, &memSizeBytes, &memSizeBytesLen, NULL, 0);
    m_ramTotalMB = static_cast<int>(memSizeBytes / 1024 / 1024);

    m_osType = "MACOS";

    // CPU frequency — Apple Silicon doesn't expose these via sysctl, so fall back
    // to reasonable defaults for Apple M-series chips.
    m_cpuClockMin = getCpuFreqGHz("hw.cpufrequency_min");
    m_cpuClockMax = getCpuFreqGHz("hw.cpufrequency_max");
    if (m_cpuClockMax <= 0.0) {
        // Apple Silicon: sysctl doesn't expose frequency.
        // Use known M-series ranges (E-core min, P-core max).
        m_cpuClockMin = 0.6;
        m_cpuClockMax = (m_numCPUs >= 10) ? 4.05 : 3.49; // M Pro/Max vs base
    }

    // Fetch boot time once
    mib[0] = CTL_KERN;
    mib[1] = KERN_BOOTTIME;
    struct timeval boottime;
    size_t boottimeLen = sizeof(boottime);
    if (sysctl(mib, 2U, &boottime, &boottimeLen, NULL, 0) == 0) {
        m_bootTimeSecs = boottime.tv_sec;
    }

    connect(&m_timer, &QTimer::timeout, this, &SystemMonitorMacOS::performUpdate);
    m_timer.start(1000);
}

SystemMonitorMacOS::~SystemMonitorMacOS() {
    if (m_prevCpuInfo) {
        size_t prevCpuInfoSize = sizeof(integer_t) * m_numPrevCpuInfo;
        vm_deallocate(mach_task_self(), (vm_address_t)m_prevCpuInfo, prevCpuInfoSize);
    }
}

void SystemMonitorMacOS::update() {
    performUpdate();
}

void SystemMonitorMacOS::performUpdate() {
    updateCpuUsage();
    updateMemoryUsage();
    updateLoadAverage();
    updateTemperature();
    updateUptime();
    updatePowerSource();
    updateTotalTasks();
    emit dataUpdated();
}

void SystemMonitorMacOS::updateCpuUsage() {
    processor_info_array_t cpuInfo;
    mach_msg_type_number_t numCpuInfo;
    natural_t numCPUsU;

    kern_return_t err = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &numCPUsU,
                                            &cpuInfo, &numCpuInfo);

    if (err == KERN_SUCCESS) {
        if (m_prevCpuInfo) {
            m_cpuUsagePercent.resize(m_numCPUs);
            long long globalTotals = 0;
            long long globalIdles = 0;
            long long globalUser = 0;
            long long globalSystem = 0;
            for (unsigned int i = 0; i < m_numCPUs; ++i) {
                long long user = cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_USER] -
                                 m_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_USER] +
                                 (cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_NICE] -
                                  m_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_NICE]);
                long long system = cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM] -
                                   m_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM];
                long long idle = cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_IDLE] -
                                 m_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_IDLE];
                long long total = user + system + idle;
                m_cpuUsagePercent[i] =
                    (total == 0) ? 0.0 : static_cast<double>(user + system) / total * 100.0;
                globalTotals += total;
                globalIdles += idle;
                globalUser += user;
                globalSystem += system;
            }
            if (globalTotals > 0) {
                m_cpuUsageTotal =
                    static_cast<double>(globalTotals - globalIdles) / globalTotals * 100.0;
                m_cpuUsageUser = static_cast<double>(globalUser) / globalTotals * 100.0;
                m_cpuUsageSystem = static_cast<double>(globalSystem) / globalTotals * 100.0;
            } else {
                m_cpuUsageTotal = 0.0;
                m_cpuUsageUser = 0.0;
                m_cpuUsageSystem = 0.0;
            }

            size_t prevCpuInfoSize = sizeof(integer_t) * m_numPrevCpuInfo;
            vm_deallocate(mach_task_self(), (vm_address_t)m_prevCpuInfo, prevCpuInfoSize);
        }

        m_prevCpuInfo = cpuInfo;
        m_numPrevCpuInfo = numCpuInfo;
    }
}

void SystemMonitorMacOS::updateMemoryUsage() {
    vm_size_t page_size;
    mach_port_t mach_port = mach_host_self();
    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = sizeof(vm_stats) / sizeof(natural_t);

    if (host_page_size(mach_port, &page_size) == KERN_SUCCESS &&
        host_statistics64(mach_port, HOST_VM_INFO, (host_info64_t)&vm_stats, &count) ==
            KERN_SUCCESS) {

        long long used_memory = ((int64_t)vm_stats.active_count + (int64_t)vm_stats.inactive_count +
                                 (int64_t)vm_stats.wire_count) *
                                (int64_t)page_size;

        m_ramUsedMB = static_cast<int>(used_memory / 1024 / 1024);
    }
}

void SystemMonitorMacOS::updateLoadAverage() {
    struct loadavg load;
    size_t size = sizeof(load);
    if (sysctlbyname("vm.loadavg", &load, &size, NULL, 0) == 0) {
        m_loadAverage1m = (double)load.ldavg[0] / load.fscale;
    }
}

void SystemMonitorMacOS::updateTemperature() {
    static io_connect_t smcConn = openSMC();
    if (smcConn == IO_OBJECT_NULL) {
        m_cpuTempCelsius = -1.0;
        return;
    }

    // CPU temperature keys in priority order:
    // Apple Silicon: Tp09 = P-cluster, Tp01 = E-cluster, Tp0D = die
    // Intel: TC0P = proximity, TC0D = die
    static const char *keys[] = {"Tp09", "Tp0D", "Tp01", "TC0P", "TC0D"};
    for (auto *key : keys) {
        double temp = readSMCTemp(smcConn, key);
        if (temp > 0.0 && temp < 130.0) {
            m_cpuTempCelsius = temp;
            return;
        }
    }
    m_cpuTempCelsius = -1.0; // SMC available but no valid reading
}

void SystemMonitorMacOS::updateUptime() {
    if (m_bootTimeSecs == 0)
        return;
    time_t uptimeSecs = time(NULL) - m_bootTimeSecs;
    int days = static_cast<int>(uptimeSecs / 86400);
    int hours = static_cast<int>((uptimeSecs % 86400) / 3600);
    int mins = static_cast<int>((uptimeSecs % 3600) / 60);
    m_uptime =
        QString("%1:%2:%3").arg(days).arg(hours, 2, 10, QChar('0')).arg(mins, 2, 10, QChar('0'));
}

void SystemMonitorMacOS::updatePowerSource() {
    CFTypeRef psInfo = IOPSCopyPowerSourcesInfo();
    if (!psInfo) {
        m_powerSource = "UNKNOWN";
        return;
    }

    CFArrayRef psList = IOPSCopyPowerSourcesList(psInfo);
    if (!psList || CFArrayGetCount(psList) == 0) {
        m_powerSource = "AC"; // Desktop Mac, no battery
        if (psList)
            CFRelease(psList);
        CFRelease(psInfo);
        return;
    }

    CFDictionaryRef ps = IOPSGetPowerSourceDescription(psInfo, CFArrayGetValueAtIndex(psList, 0));
    if (ps) {
        CFStringRef powerState =
            (CFStringRef)CFDictionaryGetValue(ps, CFSTR(kIOPSPowerSourceStateKey));
        if (powerState &&
            CFStringCompare(powerState, CFSTR(kIOPSACPowerValue), 0) == kCFCompareEqualTo) {
            m_powerSource = "AC";
        } else {
            // On battery — read capacity percentage
            CFNumberRef capRef =
                (CFNumberRef)CFDictionaryGetValue(ps, CFSTR(kIOPSCurrentCapacityKey));
            int capacity = 0;
            if (capRef)
                CFNumberGetValue(capRef, kCFNumberIntType, &capacity);
            m_powerSource = QString("BAT %1%").arg(capacity);
        }
    }

    CFRelease(psList);
    CFRelease(psInfo);
}

void SystemMonitorMacOS::updateTotalTasks() {
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    size_t size = 0;
    if (sysctl(mib, 4, nullptr, &size, nullptr, 0) == 0 && size > 0) {
        m_totalTasks = static_cast<int>(size / sizeof(kinfo_proc));
    }
}
