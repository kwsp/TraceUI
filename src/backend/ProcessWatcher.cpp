#include "ProcessWatcher.h"

ProcessWatcher::ProcessWatcher(QObject* parent) : QObject(parent) {}

ProcessListModel* ProcessWatcher::processes() { return &m_processModel; }

QVariantList ProcessWatcher::watchedServices() const { return m_watchedServices; }

void ProcessWatcher::updateProcesses(QList<ProcessEntry> incoming) {
    m_processModel.updateData(std::move(incoming));
}
