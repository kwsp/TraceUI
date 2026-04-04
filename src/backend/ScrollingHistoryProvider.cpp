#include "ScrollingHistoryProvider.h"

#include <QImage>
#include <QQuickWindow>
#include <QSGNode>
#include <algorithm>
#include <limits>

static constexpr int kUint8Max = std::numeric_limits<uint8_t>::max();
static constexpr float kScaleThreshold = 1e-5F;

ScrollingHistoryProvider::ScrollingHistoryProvider(QQuickItem *parent)
    : QQuickItem(parent), m_img(kSize, 1, QImage::Format_RGBA8888) {
  setFlag(ItemHasContents, true);
}

ScrollingHistoryProvider::~ScrollingHistoryProvider() = default;

QSGTextureProvider *ScrollingHistoryProvider::textureProvider() const {
  if (m_provider == nullptr)
    m_provider = new HistoryTextureProvider;
  return m_provider;
}

void ScrollingHistoryProvider::pushSample(float ch0, float ch1,
                                          float minMaxRef) {
  {
    std::lock_guard lock(m_mutex);
    m_history0.at(m_writeIndex) = ch0;
    m_history1.at(m_writeIndex) = ch1;
    m_writeIndex = (m_writeIndex + 1) % kSize;
    emit phaseChanged();
    m_dirty = true;

    float maxVal = minMaxRef;
    for (float v : m_history0)
      maxVal = std::max(maxVal, v);
    for (float v : m_history1)
      maxVal = std::max(maxVal, v);

    m_graphMax = maxVal;

    if (std::abs(m_graphScale - 1.0F) > kScaleThreshold) {
      m_graphScale = 1.0F;
      emit graphScaleChanged();
    }
  }
  update();
}

QSGNode *
ScrollingHistoryProvider::updatePaintNode(QSGNode *oldNode,
                                          UpdatePaintNodeData * /*data*/) {
  if (m_provider == nullptr)
    m_provider = new HistoryTextureProvider;

  if (!m_dirty)
    return oldNode;

  {
    std::lock_guard lock(m_mutex);
    for (int i = 0; i < kSize; i++) {
      int idx = (m_writeIndex + i) % kSize;
      float n0 = std::clamp(m_history0.at(idx) / m_graphMax, 0.0F, 1.0F);
      float n1 = std::clamp(m_history1.at(idx) / m_graphMax, 0.0F, 1.0F);
      m_img.setPixel(i, 0,
                     qRgba(static_cast<int>(n0 * kUint8Max),
                           static_cast<int>(n1 * kUint8Max), 0, kUint8Max));
    }
    m_dirty = false;
  }

  delete m_provider->m_texture;
  m_provider->m_texture =
      window()->createTextureFromImage(m_img, QQuickWindow::TextureIsOpaque);
  emit m_provider->textureChanged();

  return oldNode;
}

void ScrollingHistoryProvider::releaseResources() {
  if (m_provider != nullptr) {
    delete m_provider->m_texture;
    m_provider->m_texture = nullptr;
    delete m_provider;
    m_provider = nullptr;
  }
}
