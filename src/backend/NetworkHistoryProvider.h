#pragma once

#include "ScrollingHistoryProvider.h"

class NetworkHistoryProvider : public ScrollingHistoryProvider {
    Q_OBJECT
    QML_ELEMENT

    static constexpr float kMinMaxReference = 1048.576F; // 1 KB/s floor to avoid noise zooming

public:
    explicit NetworkHistoryProvider(QQuickItem *parent = nullptr);

    Q_INVOKABLE void onDataUpdated(double dlBytesPerSec, double ulBytesPerSec);
};
