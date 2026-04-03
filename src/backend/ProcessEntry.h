#pragma once
#include <QString>

struct ProcessEntry {
  QString name;
  int pid = 0;
  int cpuPct = 0;
  int ramMB = 0;
};
