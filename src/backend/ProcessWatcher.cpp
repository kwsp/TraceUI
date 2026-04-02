#include "ProcessWatcher.h"

ProcessWatcher::ProcessWatcher(QObject* parent) : QObject(parent) {}

[[nodiscard]] QVariantList ProcessWatcher::processes() const { return m_processes; }
[[nodiscard]] QVariantList ProcessWatcher::watchedServices() const { return m_watchedServices; }
