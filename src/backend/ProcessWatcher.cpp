#include "ProcessWatcher.h"

ProcessWatcher::ProcessWatcher(QObject* parent) : QObject(parent) {}

QVariantList ProcessWatcher::processes() const { return m_processes; }
QVariantList ProcessWatcher::watchedServices() const { return m_watchedServices; }
