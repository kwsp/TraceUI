#include "NetworkHistoryProvider.h"

#include <QImage>
#include <QQuickWindow>
#include <QSGNode>
#include <algorithm>
#include <limits>

static constexpr int kUint8Max = std::numeric_limits<uint8_t>::max();

NetworkHistoryProvider::NetworkHistoryProvider(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
}

NetworkHistoryProvider::~NetworkHistoryProvider() = default;

QSGTextureProvider* NetworkHistoryProvider::textureProvider() const
{
    if (m_provider == nullptr)
        m_provider = new HistoryTextureProvider;
    return m_provider;
}

void NetworkHistoryProvider::onDataUpdated(double dl, double ul)
{
    {
        std::lock_guard lock(m_mutex);
        m_dlHistory.at(m_writeIndex) = std::clamp(static_cast<float>(dl) / kMaxBytesPerSec, 0.F, 1.F);
        m_ulHistory.at(m_writeIndex) = std::clamp(static_cast<float>(ul) / kMaxBytesPerSec, 0.F, 1.F);
        m_writeIndex = (m_writeIndex + 1) % kSize;
        m_dirty = true;
    }
    update();
}

QSGNode* NetworkHistoryProvider::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* /*data*/)
{
    if (m_provider == nullptr)
        m_provider = new HistoryTextureProvider;

    if (!m_dirty)
        return oldNode;

    QImage img(kSize, 1, QImage::Format_RGBA8888);
    {
        std::lock_guard lock(m_mutex);
        for (int i = 0; i < kSize; i++) {
            int idx = (m_writeIndex + i) % kSize; // oldest → newest, left → right
            img.setPixel(i, 0, qRgba(
                static_cast<int>(m_dlHistory.at(idx) * kUint8Max),
                static_cast<int>(m_ulHistory.at(idx) * kUint8Max),
                0, kUint8Max));
        }
        m_dirty = false;
    }

    delete m_provider->m_texture;
    m_provider->m_texture = window()->createTextureFromImage(img, QQuickWindow::TextureIsOpaque);
    emit m_provider->textureChanged();

    return oldNode; // this item has no visual node of its own
}

void NetworkHistoryProvider::releaseResources()
{
    if (m_provider != nullptr) {
        delete m_provider->m_texture;
        m_provider->m_texture = nullptr;
        delete m_provider;
        m_provider = nullptr;
    }
}
