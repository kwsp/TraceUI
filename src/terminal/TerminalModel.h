#pragma once

#include <QAbstractListModel>
#include "TerminalBackend.h"

class TerminalModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(TerminalBackend* backend READ backend WRITE setBackend NOTIFY backendChanged)

public:
    enum Role {
        TextRole = Qt::UserRole + 1,
        ForegroundRole,
        BackgroundRole,
    };

    explicit TerminalModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] TerminalBackend *backend() const { return m_backend; }
    void setBackend(TerminalBackend *backend);

signals:
    void backendChanged();

private slots:
    void onScreenDamaged(int startRow, int endRow);

private:
    TerminalBackend *m_backend = nullptr;
};
