#pragma once

#include "ScrollingHistoryProvider.h"

class CpuHistoryProvider : public ScrollingHistoryProvider {
    Q_OBJECT
    QML_ELEMENT

public:
    explicit CpuHistoryProvider(QQuickItem *parent = nullptr);

    // ch0 = user %, ch1 = system % (both 0-100)
    Q_INVOKABLE void onDataUpdated(double userPct, double systemPct);
};
