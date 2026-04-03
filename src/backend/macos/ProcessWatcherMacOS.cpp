#include "ProcessWatcherMacOS.h"
#include "backend/ProcessEntry.h"
#include <algorithm>
#include <libproc.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <vector>

static constexpr int kMaxProcesses = 20;
static constexpr uint64_t kBytesPerMB = 1024ULL * 1024;

ProcessWatcherMacOS::ProcessWatcherMacOS(QObject *parent)
    : ProcessWatcher(parent) {
  mach_timebase_info(&m_timebase);
  m_coreCount = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
  if (m_coreCount < 1)
    m_coreCount = 1;
  m_pollTimer.start();
  connect(&m_timer, &QTimer::timeout, this,
          &ProcessWatcherMacOS::performUpdate);
  connect(this, &ProcessWatcher::sortByCpuChanged, this,
          &ProcessWatcherMacOS::sortAndPublish);
  m_timer.start(3000);
}

void ProcessWatcherMacOS::update() { performUpdate(); }

void ProcessWatcherMacOS::sortAndPublish() {
  if (sortByCpu())
    std::sort(m_allEntries.begin(), m_allEntries.end(),
              [](const ProcessEntry &a, const ProcessEntry &b) {
                return a.cpuPct > b.cpuPct;
              });
  else
    std::sort(m_allEntries.begin(), m_allEntries.end(),
              [](const ProcessEntry &a, const ProcessEntry &b) {
                return a.ramMB > b.ramMB;
              });

  const int limit =
      std::min(static_cast<int>(m_allEntries.size()), kMaxProcesses);
  QList<ProcessEntry> result;
  result.reserve(limit);
  for (int i = 0; i < limit; ++i)
    result.append(m_allEntries[i]);

  updateProcesses(std::move(result));
}

void ProcessWatcherMacOS::performUpdate() {
  const auto elapsedNs = static_cast<uint64_t>(m_pollTimer.nsecsElapsed());
  m_pollTimer.restart();

  int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
  size_t size = 0;
  if (sysctl(mib, 4, nullptr, &size, nullptr, 0) != 0 || size == 0)
    return;

  std::vector<kinfo_proc> procs(size / sizeof(kinfo_proc) + 4);
  if (sysctl(mib, 4, procs.data(), &size, nullptr, 0) != 0)
    return;

  const size_t procCount = size / sizeof(kinfo_proc);

  QHash<pid_t, uint64_t> newCpuTimes;
  newCpuTimes.reserve(procCount);

  m_allEntries.clear();
  m_allEntries.reserve(procCount);

  for (size_t i = 0; i < procCount; ++i) {
    const kinfo_proc &kp = procs[i];
    const pid_t pid = kp.kp_proc.p_pid;
    if (pid == 0)
      continue;

    proc_taskinfo pti{};
    if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &pti, sizeof(pti)) !=
        sizeof(pti))
      continue;

    const uint64_t totalCpu = pti.pti_total_user + pti.pti_total_system;
    newCpuTimes[pid] = totalCpu;

    double cpuPct = 0.0;
    if (elapsedNs > 0 && m_cpuTimes.contains(pid)) {
      const uint64_t delta = totalCpu - m_cpuTimes[pid];
      const double deltaNs =
          static_cast<double>(delta) * m_timebase.numer / m_timebase.denom;
      cpuPct = std::clamp(deltaNs / static_cast<double>(elapsedNs) * 100.0, 0.0,
                          100.0 * m_coreCount);
    }

    if (!m_nameCache.contains(pid)) {
      char buf[PROC_PIDPATHINFO_MAXSIZE];
      if (proc_pidpath(pid, buf, sizeof(buf)) > 0)
        m_nameCache[pid] = QString::fromUtf8(buf).section('/', -1);
      else
        m_nameCache[pid] = QString::fromUtf8(kp.kp_proc.p_comm);
      if (m_nameCache[pid].isEmpty())
        m_nameCache[pid] = QStringLiteral("Unknown");
    }

    ProcessEntry entry;
    entry.pid = pid;
    entry.name = m_nameCache[pid];
    entry.ramMB = static_cast<int>(pti.pti_resident_size / kBytesPerMB);
    entry.cpuPct = static_cast<int>(cpuPct);
    m_allEntries.append(std::move(entry));
  }

  for (auto it = m_nameCache.begin(); it != m_nameCache.end();) {
    it = newCpuTimes.contains(it.key()) ? std::next(it) : m_nameCache.erase(it);
  }

  m_cpuTimes = std::move(newCpuTimes);

  sortAndPublish();
  emit dataUpdated();
}
