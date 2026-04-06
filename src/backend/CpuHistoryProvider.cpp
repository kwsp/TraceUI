#include "CpuHistoryProvider.h"

// CPU percentages are 0-100; use 100 as the fixed reference so the graph
// always spans the full 0-100% range without auto-scaling.
static constexpr float kMaxReference = 100.0F;

CpuHistoryProvider::CpuHistoryProvider(QQuickItem *parent) : ScrollingHistoryProvider(parent) {}

void CpuHistoryProvider::onDataUpdated(double userPct, double systemPct) {
    pushSample(static_cast<float>(userPct), static_cast<float>(systemPct), kMaxReference);
}
