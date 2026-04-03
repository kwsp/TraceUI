#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

class NetworkMonitor : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("NetworkMonitor is provided by the backend")
    Q_PROPERTY(double downloadBytesPerSec READ downloadBytesPerSec NOTIFY dataUpdated)
    Q_PROPERTY(double uploadBytesPerSec READ uploadBytesPerSec NOTIFY dataUpdated)
    Q_PROPERTY(int activeConnections READ activeConnections NOTIFY dataUpdated)
    Q_PROPERTY(QString mainInterface READ mainInterface NOTIFY dataUpdated)
    Q_PROPERTY(bool vpnActive READ vpnActive NOTIFY dataUpdated)

public:
    explicit NetworkMonitor(QObject* parent = nullptr);
    virtual ~NetworkMonitor() = default;

    [[nodiscard]] double  downloadBytesPerSec() const;
    [[nodiscard]] double  uploadBytesPerSec() const;
    [[nodiscard]] int     activeConnections() const;
    [[nodiscard]] QString mainInterface() const;
    [[nodiscard]] bool    vpnActive() const;

    virtual void update() = 0;

signals:
    void dataUpdated();

protected:
    double m_downloadBytesPerSec = 0.0;
    double m_uploadBytesPerSec = 0.0;
    int m_activeConnections = 0;
    QString m_interface = "en0";
    bool m_vpnActive = false;
};
