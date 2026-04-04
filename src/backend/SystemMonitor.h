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
    Q_PROPERTY(double cpuUsageTotal READ cpuUsageTotal NOTIFY dataUpdated)
    Q_PROPERTY(double cpuUsageUser READ cpuUsageUser NOTIFY dataUpdated)
    Q_PROPERTY(double cpuUsageSystem READ cpuUsageSystem NOTIFY dataUpdated)
    Q_PROPERTY(double loadAverage1m READ loadAverage1m NOTIFY dataUpdated)
    Q_PROPERTY(QString cpuName READ cpuName CONSTANT)
    Q_PROPERTY(QString uptime READ uptime NOTIFY dataUpdated)

public:
    explicit SystemMonitor(QObject* parent = nullptr);
    SystemMonitor(const SystemMonitor &) = delete;
    SystemMonitor &operator=(const SystemMonitor &) = delete;
    SystemMonitor(SystemMonitor &&) = delete;
    SystemMonitor &operator=(SystemMonitor &&) = delete;
    virtual ~SystemMonitor() = default;

    [[nodiscard]] QList<double> cpuUsagePercent() const;
    [[nodiscard]] int ramUsedMB() const;
    [[nodiscard]] int ramTotalMB() const;
    [[nodiscard]] int swapUsedMB() const;
    [[nodiscard]] double cpuTempCelsius() const;
    [[nodiscard]] double loadAverage1m() const;
    [[nodiscard]] double cpuUsageTotal() const;
    [[nodiscard]] double cpuUsageUser() const;
    [[nodiscard]] double cpuUsageSystem() const;
    [[nodiscard]] QString cpuName() const;
    [[nodiscard]] QString uptime() const;

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
    double m_cpuUsageTotal = 0.0;
    double m_cpuUsageUser = 0.0;
    double m_cpuUsageSystem = 0.0;
    QString m_cpuName;
    QString m_uptime;
};
