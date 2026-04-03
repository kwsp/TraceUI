#include "ProcessWatcherMacOS.h"
#include "backend/ProcessEntry.h"
#include <libproc.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <algorithm>
#include <vector>

static constexpr int      kMaxProcesses = 20;
static constexpr uint64_t kBytesPerMB   = 1024ULL * 1024;

ProcessWatcherMacOS::ProcessWatcherMacOS(QObject* parent)
    : ProcessWatcher(parent) {
    mach_timebase_info(&m_timebase);
    m_coreCount = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
    if (m_coreCount < 1) m_coreCount = 1;
    m_pollTimer.start();
    connect(&m_timer, &QTimer::timeout, this, &ProcessWatcherMacOS::performUpdate);
    m_timer.start(3000);
}

void ProcessWatcherMacOS::update() {
    performUpdate();
}

void ProcessWatcherMacOS::performUpdate() {
    // Elapsed time since last poll — single denominator for all CPU% calculations this tick
    const uint64_t elapsedNs = static_cast<uint64_t>(m_pollTimer.nsecsElapsed());
    m_pollTimer.restart();

    // One bulk sysctl to get all kinfo_proc structs — replaces proc_listpids loop
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    size_t size = 0;
    if (sysctl(mib, 4, nullptr, &size, nullptr, 0) != 0 || size == 0)
        return;

    std::vector<kinfo_proc> procs(size / sizeof(kinfo_proc) + 4);
    if (sysctl(mib, 4, procs.data(), &size, nullptr, 0) != 0)
        return;

    const int procCount = static_cast<int>(size / sizeof(kinfo_proc));

    std::vector<ProcessEntry> entries;
    entries.reserve(procCount);

    QHash<pid_t, uint64_t> newCpuTimes;
    newCpuTimes.reserve(procCount);

    for (int i = 0; i < procCount; ++i) {
        const kinfo_proc& kp = procs[i];
        const pid_t pid = kp.kp_proc.p_pid;
        if (pid == 0) continue;

        // Fetch per-process CPU time and memory — lightweight, always needed
        proc_taskinfo pti{};
        if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &pti, sizeof(pti)) != sizeof(pti))
            continue;

        const uint64_t totalCpu = pti.pti_total_user + pti.pti_total_system;
        newCpuTimes[pid] = totalCpu;

        // CPU%: delta of accumulated process CPU time / elapsed wall time
        double cpuPct = 0.0;
        if (elapsedNs > 0 && m_cpuTimes.contains(pid)) {
            const uint64_t delta   = totalCpu - m_cpuTimes[pid];
            const double   deltaNs = static_cast<double>(delta) * m_timebase.numer / m_timebase.denom;
            cpuPct = std::clamp(deltaNs / static_cast<double>(elapsedNs) * 100.0, 0.0, 100.0 * m_coreCount);
        }

        // Name: call proc_pidpath only for new PIDs; use kp_proc.p_comm as fallback.
        // Both are cached so subsequent polls pay no syscall cost for this process.
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
        entry.pid    = pid;
        entry.name   = m_nameCache[pid];
        entry.ramMB  = static_cast<int>(pti.pti_resident_size / kBytesPerMB);
        entry.cpuPct = static_cast<int>(cpuPct);
        entries.push_back(std::move(entry));
    }

    // Evict cached names for processes that no longer exist
    for (auto it = m_nameCache.begin(); it != m_nameCache.end(); ) {
        it = newCpuTimes.contains(it.key()) ? std::next(it) : m_nameCache.erase(it);
    }
    m_cpuTimes = std::move(newCpuTimes);

    // Sort by RAM descending, keep top N
    std::sort(entries.begin(), entries.end(),
              [](const ProcessEntry& a, const ProcessEntry& b) { return a.ramMB > b.ramMB; });

    const int limit = std::min(static_cast<int>(entries.size()), kMaxProcesses);
    QList<ProcessEntry> result;
    result.reserve(limit);
    for (int i = 0; i < limit; ++i)
        result.append(std::move(entries[i]));

    updateProcesses(std::move(result));
    emit dataUpdated();
}
