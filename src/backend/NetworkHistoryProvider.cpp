#include "NetworkHistoryProvider.h"

#include <QImage>
#include <QQuickWindow>
#include <QSGNode>
#include <algorithm>
#include <limits>

static constexpr int kUint8Max = std::numeric_limits<uint8_t>::max();

NetworkHistoryProvider::NetworkHistoryProvider(QQuickItem *parent)
    : QQuickItem(parent) {
  setFlag(ItemHasContents, true);
}

NetworkHistoryProvider::~NetworkHistoryProvider() = default;

QSGTextureProvider *NetworkHistoryProvider::textureProvider() const {
  if (m_provider == nullptr)
    m_provider = new HistoryTextureProvider;
  return m_provider;
}

void NetworkHistoryProvider::onDataUpdated(double dl, double ul) {
  {
    std::lock_guard lock(m_mutex);
    m_dlHistory.at(m_writeIndex) = static_cast<float>(dl);
    m_ulHistory.at(m_writeIndex) = static_cast<float>(ul);
    m_writeIndex = (m_writeIndex + 1) % kSize;
    emit phaseChanged();
    m_dirty = true;

    float maxVal = NetworkHistoryProvider::kMinMaxReference;
    for (float val : m_dlHistory)
      maxVal = std::max(maxVal, val);
    for (float val : m_ulHistory)
      maxVal = std::max(maxVal, val);

    // We now use m_graphMax to normalize in the texture itself for best
    // precision.
    m_graphMax = maxVal;

    // We keep m_graphScale at 1.0 since the texture is now perfectly normalized
    // to its peak.
    if (std::abs(m_graphScale - 1.0F) >
        NetworkHistoryProvider::kScaleThreshold) {
      m_graphScale = 1.0F;
      emit graphScaleChanged();
    }
  }
  update();
}

QSGNode *
NetworkHistoryProvider::updatePaintNode(QSGNode *oldNode,
                                        UpdatePaintNodeData * /*data*/) {
  if (m_provider == nullptr)
    m_provider = new HistoryTextureProvider;

  if (!m_dirty)
    return oldNode;

  QImage img(kSize, 1, QImage::Format_RGBA8888);
  {
    std::lock_guard lock(m_mutex);
    for (int i = 0; i < kSize; i++) {
      int idx = (m_writeIndex + i) % kSize;
      float dlNorm = std::clamp(m_dlHistory.at(idx) / m_graphMax, 0.0F, 1.0F);
      float ulNorm = std::clamp(m_ulHistory.at(idx) / m_graphMax, 0.0F, 1.0F);
      img.setPixel(i, 0,
                   qRgba(static_cast<int>(dlNorm * kUint8Max),
                         static_cast<int>(ulNorm * kUint8Max), 0, kUint8Max));
    }
    m_dirty = false;
  }

  delete m_provider->m_texture;
  m_provider->m_texture =
      window()->createTextureFromImage(img, QQuickWindow::TextureIsOpaque);
  emit m_provider->textureChanged();

  return oldNode; // this item has no visual node of its own
}

void NetworkHistoryProvider::releaseResources() {
  if (m_provider != nullptr) {
    delete m_provider->m_texture;
    m_provider->m_texture = nullptr;
    delete m_provider;
    m_provider = nullptr;
  }
}
