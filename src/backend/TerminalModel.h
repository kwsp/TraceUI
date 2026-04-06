#pragma once

#include <QAbstractListModel>
#include <vterm.h>
#include "TerminalBackend.h"

class TerminalModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(TerminalBackend* backend READ backend WRITE setBackend NOTIFY backendChanged)

public:
    enum Roles {
        TextRole = Qt::UserRole + 1,
        ForegroundRole,
        BackgroundRole
    };

    explicit TerminalModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    TerminalBackend* backend() const { return m_backend; }
    void setBackend(TerminalBackend* backend);

signals:
    void backendChanged();

private slots:
    void updateScreen();

private:
    TerminalBackend* m_backend = nullptr;
};
