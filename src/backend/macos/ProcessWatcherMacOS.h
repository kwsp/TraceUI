#pragma once

#include "backend/ProcessWatcher.h"
#include <QElapsedTimer>
#include <QHash>
#include <QList>
#include <QTimer>
#include <mach/mach_time.h>

class ProcessWatcherMacOS : public ProcessWatcher {
  Q_OBJECT

public:
  explicit ProcessWatcherMacOS(QObject *parent = nullptr);
  ProcessWatcherMacOS(const ProcessWatcherMacOS &) = delete;
  ProcessWatcherMacOS &operator=(const ProcessWatcherMacOS &) = delete;
  ProcessWatcherMacOS(ProcessWatcherMacOS &&) = delete;
  ProcessWatcherMacOS &operator=(ProcessWatcherMacOS &&) = delete;
  ~ProcessWatcherMacOS() override = default;

  void update() override;

private slots:
  void performUpdate();
  void sortAndPublish();

private:
  QTimer m_timer;
  QElapsedTimer m_pollTimer;
  QHash<pid_t, uint64_t> m_cpuTimes;
  QHash<pid_t, QString> m_nameCache;
  QList<ProcessEntry> m_allEntries;
  mach_timebase_info_data_t m_timebase{};
  int m_coreCount = 1;
};
