#include "TerminalModel.h"

TerminalModel::TerminalModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void TerminalModel::setBackend(TerminalBackend *backend) {
    if (m_backend == backend) return;

    if (m_backend) {
        disconnect(m_backend, nullptr, this, nullptr);
    }

    m_backend = backend;

    if (m_backend) {
        connect(m_backend, &TerminalBackend::screenUpdated,
                this, &TerminalModel::onScreenUpdated);
        connect(m_backend, &TerminalBackend::rowsChanged,
                this, [this]() {
            beginResetModel();
            endResetModel();
        });
    }

    emit backendChanged();
}

int TerminalModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;  // Flat list model
    return m_backend ? m_backend->rows() : 0;
}

QVariant TerminalModel::data(const QModelIndex &index, int role) const {
    if (!m_backend || !index.isValid()) return {};

    switch (role) {
    case TextRole:
        return m_backend->getLineHtml(index.row());
    default:
        return {};
    }
}

QHash<int, QByteArray> TerminalModel::roleNames() const {
    return {
        { TextRole,       "text"    },
        { ForegroundRole, "fgColor" },
        { BackgroundRole, "bgColor" },
    };
}

void TerminalModel::onScreenUpdated() {
    const auto count = rowCount();
    if (count > 0) {
        emit dataChanged(index(0), index(count - 1), { TextRole });
    }
}
