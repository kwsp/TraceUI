#pragma once

#include "backend/ProcessWatcher.h"
#include <QTimer>

class ProcessWatcherMacOS : public ProcessWatcher {
    Q_OBJECT

public:
    explicit ProcessWatcherMacOS(QObject* parent = nullptr);
    ~ProcessWatcherMacOS() override = default;

    void update() override;

private slots:
    void performUpdate();

private:
    QTimer m_timer;
};
