#pragma once

#include <QObject>
#include <QList>
#include <QtQml/qqmlregistration.h>

class SystemMonitor : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("SystemMonitor is provided by the backend")
    Q_PROPERTY(QList<double> cpuUsagePercent READ cpuUsagePercent NOTIFY dataUpdated)
    Q_PROPERTY(int ramUsedMB READ ramUsedMB NOTIFY dataUpdated)
    Q_PROPERTY(int ramTotalMB READ ramTotalMB NOTIFY dataUpdated)
    Q_PROPERTY(int swapUsedMB READ swapUsedMB NOTIFY dataUpdated)
    Q_PROPERTY(double cpuTempCelsius READ cpuTempCelsius NOTIFY dataUpdated)
    Q_PROPERTY(double loadAverage1m READ loadAverage1m NOTIFY dataUpdated)
    Q_PROPERTY(QString cpuName READ cpuName CONSTANT)

public:
    explicit SystemMonitor(QObject* parent = nullptr);
    virtual ~SystemMonitor() = default;

    [[nodiscard]] QList<double> cpuUsagePercent() const;
    [[nodiscard]] int ramUsedMB() const;
    [[nodiscard]] int ramTotalMB() const;
    [[nodiscard]] int swapUsedMB() const;
    [[nodiscard]] double cpuTempCelsius() const;
    [[nodiscard]] double loadAverage1m() const;
    [[nodiscard]] virtual QString cpuName() const = 0;

    virtual void update() = 0;

signals:
    void dataUpdated();

protected:
    QList<double> m_cpuUsagePercent;
    int m_ramUsedMB = 0;
    int m_ramTotalMB = 0;
    int m_swapUsedMB = 0;
    double m_cpuTempCelsius = 0.0;
    double m_loadAverage1m = 0.0;
};
