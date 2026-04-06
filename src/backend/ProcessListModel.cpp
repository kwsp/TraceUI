#include "ProcessListModel.h"
#include <algorithm>

ProcessListModel::ProcessListModel(QObject *parent) : QAbstractListModel(parent) {}

int ProcessListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_data.size());
}

QVariant ProcessListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_data.size())
        return {};
    const ProcessEntry &e = m_data[index.row()];
    switch (role) {
    case NameRole:
        return e.name;
    case PidRole:
        return e.pid;
    case CpuPctRole:
        return e.cpuPct;
    case RamMBRole:
        return e.ramMB;
    default:
        return {};
    }
}

QHash<int, QByteArray> ProcessListModel::roleNames() const {
    return {
        {NameRole, "name"},
        {PidRole, "pid"},
        {CpuPctRole, "cpuPct"},
        {RamMBRole, "ramMB"},
    };
}

void ProcessListModel::updateData(QList<ProcessEntry> incoming) {
    const int oldSize = static_cast<int>(m_data.size());
    const int newSize = static_cast<int>(incoming.size());
    const int common = std::min(oldSize, newSize);

    for (int i = 0; i < common; ++i) {
        if (m_data[i].name != incoming[i].name || m_data[i].pid != incoming[i].pid ||
            m_data[i].cpuPct != incoming[i].cpuPct || m_data[i].ramMB != incoming[i].ramMB) {
            m_data[i] = incoming[i];
            emit dataChanged(index(i), index(i));
        }
    }

    if (newSize > oldSize) {
        beginInsertRows({}, oldSize, newSize - 1);
        for (int i = oldSize; i < newSize; ++i)
            m_data.append(std::move(incoming[i]));
        endInsertRows();
    } else if (newSize < oldSize) {
        beginRemoveRows({}, newSize, oldSize - 1);
        m_data.resize(newSize);
        endRemoveRows();
    }
}
