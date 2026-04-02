#pragma once

#include <QObject>
#include <QVariantList>

class ProcessWatcher : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList processes READ processes NOTIFY dataUpdated)
    Q_PROPERTY(QVariantList watchedServices READ watchedServices NOTIFY dataUpdated)

public:
    explicit ProcessWatcher(QObject* parent = nullptr);
    virtual ~ProcessWatcher() = default;

    [[nodiscard]] QVariantList processes() const;
    [[nodiscard]] QVariantList watchedServices() const;

    virtual void update() = 0;

signals:
    void dataUpdated();
    void serviceDown(const QString& name);

protected:
    QVariantList m_processes;
    QVariantList m_watchedServices;
};
