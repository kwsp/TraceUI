#include "ProcessWatcher.h"

ProcessWatcher::ProcessWatcher(QObject* parent) : QObject(parent) {}

ProcessListModel* ProcessWatcher::processes() { return &m_processModel; }

QVariantList ProcessWatcher::watchedServices() const { return m_watchedServices; }

bool ProcessWatcher::sortByCpu() const { return m_sortByCpu; }

void ProcessWatcher::toggleSort() {
    m_sortByCpu = !m_sortByCpu;
    emit sortByCpuChanged();
}

void ProcessWatcher::updateProcesses(QList<ProcessEntry> incoming) {
    m_processModel.updateData(std::move(incoming));
}
