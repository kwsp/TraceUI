#pragma once

#include <QObject>
#include <QVariantList>

class SystemMonitor : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList cpuUsagePercent READ cpuUsagePercent NOTIFY dataUpdated)
    Q_PROPERTY(int ramUsedMB READ ramUsedMB NOTIFY dataUpdated)
    Q_PROPERTY(int ramTotalMB READ ramTotalMB NOTIFY dataUpdated)
    Q_PROPERTY(int swapUsedMB READ swapUsedMB NOTIFY dataUpdated)
    Q_PROPERTY(double cpuTempCelsius READ cpuTempCelsius NOTIFY dataUpdated)
    Q_PROPERTY(double loadAverage1m READ loadAverage1m NOTIFY dataUpdated)

public:
    explicit SystemMonitor(QObject* parent = nullptr);
    virtual ~SystemMonitor() = default;

    QVariantList cpuUsagePercent() const;
    int ramUsedMB() const;
    int ramTotalMB() const;
    int swapUsedMB() const;
    double cpuTempCelsius() const;
    double loadAverage1m() const;

    virtual void update() = 0;

signals:
    void dataUpdated();

protected:
    QVariantList m_cpuUsagePercent;
    int m_ramUsedMB = 0;
    int m_ramTotalMB = 0;
    int m_swapUsedMB = 0;
    double m_cpuTempCelsius = 0.0;
    double m_loadAverage1m = 0.0;
};
