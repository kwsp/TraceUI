#include "ProcessWatcherMacOS.h"
#include <QVariantMap>
#include <QVariantList>
#include <libproc.h>
#include <vector>

ProcessWatcherMacOS::ProcessWatcherMacOS(QObject* parent) 
    : ProcessWatcher(parent) {
    connect(&m_timer, &QTimer::timeout, this, &ProcessWatcherMacOS::performUpdate);
    m_timer.start(3000); // 3 seconds polling
}

void ProcessWatcherMacOS::update() {
    performUpdate();
}

void ProcessWatcherMacOS::performUpdate() {
    int numberOfProcesses = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    if (numberOfProcesses <= 0) return;

    std::vector<pid_t> pids(numberOfProcesses / sizeof(pid_t) + 10);
    numberOfProcesses = proc_listpids(PROC_ALL_PIDS, 0, pids.data(), pids.size() * sizeof(pid_t));
    if (numberOfProcesses <= 0) return;

    int pidCount = numberOfProcesses / sizeof(pid_t);
    QVariantList processesInfo;

    for (int i = 0; i < pidCount; ++i) {
        if (pids[i] == 0) continue;
        
        struct proc_taskinfo pti;
        if (proc_pidinfo(pids[i], PROC_PIDTASKINFO, 0, &pti, sizeof(pti)) == sizeof(pti)) {
            char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
            QString name;
            if (proc_pidpath(pids[i], pathBuffer, sizeof(pathBuffer)) > 0) {
                QString path = QString::fromUtf8(pathBuffer);
                name = path.section('/', -1);
            } else {
                char nameBuffer[256];
                proc_name(pids[i], nameBuffer, sizeof(nameBuffer));
                name = QString::fromUtf8(nameBuffer);
            }

            QVariantMap pInfo;
            pInfo["name"] = name.isEmpty() ? "Unknown" : name;
            pInfo["pid"] = pids[i];
            pInfo["ramMB"] = static_cast<int>(pti.pti_resident_size / 1024 / 1024);
            // In a real app we'd need multiple polls to calc CPU Pct, but this serves architecture
            pInfo["cpuPct"] = 0.0;
            pInfo["status"] = "Running";
            
            processesInfo.append(pInfo);
        }
    }

    m_processes = processesInfo;
    emit dataUpdated();
}
