#include "SystemMonitorMacOS.h"
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#include <sys/time.h>

SystemMonitorMacOS::SystemMonitorMacOS(QObject *parent)
    : SystemMonitor(parent) {
  int mib[2U] = {CTL_HW, HW_NCPU};
  size_t sizeOfNumCPUs = sizeof(m_numCPUs);
  sysctl(mib, 2U, &m_numCPUs, &sizeOfNumCPUs, NULL, 0);

  char buffer[256];
  size_t bufferlen = sizeof(buffer);
  // Apple Machine dependent API to get CPU name
  if (sysctlbyname("machdep.cpu.brand_string", &buffer, &bufferlen, NULL, 0) ==
      0) {
    m_cpuName = QString::fromUtf8(buffer);
  } else {
    m_cpuName = "Unknown CPU";
  }

  mib[0] = CTL_HW;
  mib[1] = HW_MEMSIZE;
  uint64_t memSizeBytes = 0;
  size_t memSizeBytesLen = sizeof(memSizeBytes);
  sysctl(mib, 2U, &memSizeBytes, &memSizeBytesLen, NULL, 0);
  m_ramTotalMB =
      static_cast<int>(memSizeBytes / 1024 / 1024); // Convert bytes to MB

  // Fetch boot time once — uptime is derived from this without repeated syscalls
  mib[0] = CTL_KERN;
  mib[1] = KERN_BOOTTIME;
  struct timeval boottime;
  size_t boottimeLen = sizeof(boottime);
  if (sysctl(mib, 2U, &boottime, &boottimeLen, NULL, 0) == 0) {
    m_bootTimeSecs = boottime.tv_sec;
  }

  connect(&m_timer, &QTimer::timeout, this, &SystemMonitorMacOS::performUpdate);
  m_timer.start(1000); // 1 second polling as per architecture docs
}

SystemMonitorMacOS::~SystemMonitorMacOS() {
  if (m_prevCpuInfo) {
    size_t prevCpuInfoSize = sizeof(integer_t) * m_numPrevCpuInfo;
    vm_deallocate(mach_task_self(), (vm_address_t)m_prevCpuInfo,
                  prevCpuInfoSize);
  }
}

void SystemMonitorMacOS::update() { performUpdate(); }

void SystemMonitorMacOS::performUpdate() {
  updateCpuUsage();
  updateMemoryUsage();
  updateLoadAverage();
  updateTemperature();
  updateUptime();
  emit dataUpdated();
}

void SystemMonitorMacOS::updateCpuUsage() {
  processor_info_array_t cpuInfo;
  mach_msg_type_number_t numCpuInfo;
  natural_t numCPUsU;

  kern_return_t err =
      host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &numCPUsU,
                          &cpuInfo, &numCpuInfo);

  if (err == KERN_SUCCESS) {
    if (m_prevCpuInfo) {
      m_cpuUsagePercent.resize(m_numCPUs);
      for (unsigned int i = 0; i < m_numCPUs; ++i) {
        integer_t inUse =
            (cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_USER] -
             m_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_USER]) +
            (cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM] -
             m_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM]) +
            (cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_NICE] -
             m_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_NICE]);
        integer_t total =
            inUse + (cpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_IDLE] -
                     m_prevCpuInfo[(CPU_STATE_MAX * i) + CPU_STATE_IDLE]);
        m_cpuUsagePercent[i] =
            (total == 0) ? 0.0 : static_cast<double>(inUse) / total * 100.0;
      }

      size_t prevCpuInfoSize = sizeof(integer_t) * m_numPrevCpuInfo;
      vm_deallocate(mach_task_self(), (vm_address_t)m_prevCpuInfo,
                    prevCpuInfoSize);
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
      host_statistics64(mach_port, HOST_VM_INFO, (host_info64_t)&vm_stats,
                        &count) == KERN_SUCCESS) {

    long long free_memory = (int64_t)vm_stats.free_count * (int64_t)page_size;
    long long used_memory =
        ((int64_t)vm_stats.active_count + (int64_t)vm_stats.inactive_count +
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
  // Stubbed until IOKit implementation
  m_cpuTempCelsius = 45.0;
}

void SystemMonitorMacOS::updateUptime() {
  if (m_bootTimeSecs == 0)
    return;
  time_t uptimeSecs = time(NULL) - m_bootTimeSecs;
  int days = static_cast<int>(uptimeSecs / 86400);
  int hours = static_cast<int>((uptimeSecs % 86400) / 3600);
  int mins = static_cast<int>((uptimeSecs % 3600) / 60);
  m_uptime = QString("%1:%2:%3")
                 .arg(days)
                 .arg(hours, 2, 10, QChar('0'))
                 .arg(mins, 2, 10, QChar('0'));
}
