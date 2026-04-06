#include "TerminalModel.h"
#include <QColor>

TerminalModel::TerminalModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void TerminalModel::setBackend(TerminalBackend* backend) {
    if (m_backend != backend) {
        if (m_backend) disconnect(m_backend, nullptr, this, nullptr);
        m_backend = backend;
        if (m_backend) {
            connect(m_backend, &TerminalBackend::screenUpdated, this, &TerminalModel::updateScreen);
            connect(m_backend, &TerminalBackend::rowsChanged, this, [this](){
                beginResetModel();
                endResetModel();
            });
        }
        emit backendChanged();
    }
}

int TerminalModel::rowCount(const QModelIndex &parent) const {
    return m_backend ? m_backend->rows() : 0;
}

QVariant TerminalModel::data(const QModelIndex &index, int role) const {
    if (!m_backend || !index.isValid()) return QVariant();

    int row = index.row();
    
    if (role == TextRole) {
        return m_backend->getLineText(row);
    }
    
    return QVariant();
}

QHash<int, QByteArray> TerminalModel::roleNames() const {
    return {
        {TextRole, "text"},
        {ForegroundRole, "fgColor"},
        {BackgroundRole, "bgColor"}
    };
}

void TerminalModel::updateScreen() {
    // For now, just reset everything when something changes.
    // In a real implementation, we would use dataChanged() for specific regions.
    emit dataChanged(index(0), index(rowCount() - 1));
}
