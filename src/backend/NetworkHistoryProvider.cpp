#include "NetworkHistoryProvider.h"

NetworkHistoryProvider::NetworkHistoryProvider(QQuickItem *parent)
    : ScrollingHistoryProvider(parent) {}

void NetworkHistoryProvider::onDataUpdated(double dl, double ul) {
    pushSample(static_cast<float>(dl), static_cast<float>(ul), kMinMaxReference);
}
