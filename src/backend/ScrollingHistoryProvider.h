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
  [[nodiscard]] QSGTexture *texture() const override { return m_texture; }
  QSGTexture *m_texture = nullptr;
};

class ScrollingHistoryProvider : public QQuickItem {
  Q_OBJECT

  Q_PROPERTY(float graphScale READ graphScale NOTIFY graphScaleChanged)
  Q_PROPERTY(float phase READ phase NOTIFY phaseChanged)

public:
  static constexpr int kSize = 128;

  explicit ScrollingHistoryProvider(QQuickItem *parent = nullptr);
  ScrollingHistoryProvider(const ScrollingHistoryProvider &) = delete;
  ScrollingHistoryProvider &operator=(const ScrollingHistoryProvider &) = delete;
  ScrollingHistoryProvider(ScrollingHistoryProvider &&) = delete;
  ScrollingHistoryProvider &operator=(ScrollingHistoryProvider &&) = delete;
  ~ScrollingHistoryProvider() override;

  [[nodiscard]] bool isTextureProvider() const override { return true; }
  [[nodiscard]] QSGTextureProvider *textureProvider() const override;

  [[nodiscard]] float graphScale() const { return m_graphScale; }
  [[nodiscard]] float phase() const {
    return static_cast<float>(m_writeIndex) / static_cast<float>(kSize);
  }

signals:
  void graphScaleChanged();
  void phaseChanged();

protected:
  QSGNode *updatePaintNode(QSGNode *oldNode,
                           UpdatePaintNodeData *data) override;
  void releaseResources() override;

  // Subclasses call this to push a new sample with two channels (R, G).
  // Values are raw (not normalized) — the base class handles normalization.
  void pushSample(float ch0, float ch1, float minMaxRef);

  // Access history for subclass use
  [[nodiscard]] const std::array<float, kSize> &history0() const {
    return m_history0;
  }
  [[nodiscard]] const std::array<float, kSize> &history1() const {
    return m_history1;
  }

private:
  mutable HistoryTextureProvider *m_provider = nullptr;

  std::array<float, kSize> m_history0{};
  std::array<float, kSize> m_history1{};
  int m_writeIndex{};
  float m_graphMax{1.0F};
  float m_graphScale{1.0F};
  bool m_dirty{false};
  QImage m_img;
  std::mutex m_mutex;
};
