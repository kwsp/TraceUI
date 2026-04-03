#pragma once

#include "ProcessEntry.h"
#include <QAbstractListModel>
#include <QList>
#include <QtQml/qqmlregistration.h>

class ProcessListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ANONYMOUS

public:
    enum Roles : uint16_t {
        NameRole  = Qt::UserRole,
        PidRole,
        CpuPctRole,
        RamMBRole
    };

    explicit ProcessListModel(QObject* parent = nullptr);

    [[nodiscard]] int      rowCount(const QModelIndex& parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void updateData(QList<ProcessEntry> incoming);

private:
    QList<ProcessEntry> m_data;
};
