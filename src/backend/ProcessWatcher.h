#pragma once

#include <QObject>
#include <QVariantList>
#include "ProcessListModel.h"

class ProcessWatcher : public QObject {
    Q_OBJECT
    Q_PROPERTY(ProcessListModel* processes READ processes CONSTANT)
    Q_PROPERTY(QVariantList watchedServices READ watchedServices NOTIFY dataUpdated)
    Q_PROPERTY(bool sortByCpu READ sortByCpu NOTIFY sortByCpuChanged)

public:
    explicit ProcessWatcher(QObject* parent = nullptr);
    virtual ~ProcessWatcher() = default;

    [[nodiscard]] ProcessListModel* processes();
    [[nodiscard]] QVariantList watchedServices() const;
    [[nodiscard]] bool sortByCpu() const;

    virtual void update() = 0;
    Q_INVOKABLE void toggleSort();

signals:
    void dataUpdated();
    void serviceDown(const QString& name);
    void sortByCpuChanged();

protected:
    void updateProcesses(QList<ProcessEntry> incoming);
    QVariantList m_watchedServices;

private:
    ProcessListModel m_processModel;
    bool m_sortByCpu = false;
};
