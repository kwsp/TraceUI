#pragma once

#include <QQuickItem>
#include <QSGTextureProvider>
#include <QtQml/qqmlregistration.h>
#include <array>
#include <mutex>

class QSGTexture;

class HistoryTextureProvider : public QSGTextureProvider {
    Q_OBJECT
public:
    [[nodiscard]] QSGTexture* texture() const override { return m_texture; }
    QSGTexture* m_texture = nullptr;
};

class NetworkHistoryProvider : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT

    static constexpr int   kSize           = 128;
    static constexpr float kMaxBytesPerSec = 102400.0F; // 100 KB/s = full scale

public:
    explicit NetworkHistoryProvider(QQuickItem* parent = nullptr);
    NetworkHistoryProvider(const NetworkHistoryProvider&)            = delete;
    NetworkHistoryProvider& operator=(const NetworkHistoryProvider&) = delete;
    NetworkHistoryProvider(NetworkHistoryProvider&&)                 = delete;
    NetworkHistoryProvider& operator=(NetworkHistoryProvider&&)      = delete;
    ~NetworkHistoryProvider() override;

    [[nodiscard]] bool isTextureProvider() const override { return true; }
    [[nodiscard]] QSGTextureProvider* textureProvider() const override;

    Q_INVOKABLE void onDataUpdated(double dlBytesPerSec, double ulBytesPerSec);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;
    void releaseResources() override;

private:
    mutable HistoryTextureProvider* m_provider = nullptr;

    std::array<float, kSize> m_dlHistory{};
    std::array<float, kSize> m_ulHistory{};
    int  m_writeIndex = 0;
    bool m_dirty      = false;
    std::mutex m_mutex;
};
