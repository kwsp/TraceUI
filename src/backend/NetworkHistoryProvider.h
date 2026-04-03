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

class NetworkHistoryProvider : public QQuickItem {
  Q_OBJECT
  QML_ELEMENT

  static constexpr int kSize = 128;
  static constexpr float kMinMaxReference =
      1048.576F; // 1 KB/s floor to avoid noise zooming
  static constexpr float kEpsilon = 1e-4F;
  static constexpr float kScaleThreshold = 1e-5F;

  Q_PROPERTY(float graphScale READ graphScale NOTIFY graphScaleChanged)
  Q_PROPERTY(float phase READ phase)

public:
  explicit NetworkHistoryProvider(QQuickItem *parent = nullptr);
  NetworkHistoryProvider(const NetworkHistoryProvider &) = delete;
  NetworkHistoryProvider &operator=(const NetworkHistoryProvider &) = delete;
  NetworkHistoryProvider(NetworkHistoryProvider &&) = delete;
  NetworkHistoryProvider &operator=(NetworkHistoryProvider &&) = delete;
  ~NetworkHistoryProvider() override;

  [[nodiscard]] bool isTextureProvider() const override { return true; }
  [[nodiscard]] QSGTextureProvider *textureProvider() const override;

  [[nodiscard]] float graphScale() const { return m_graphScale; }
  [[nodiscard]] float phase() const {
    return static_cast<float>(m_writeIndex) / static_cast<float>(kSize);
  }

  Q_INVOKABLE void onDataUpdated(double dlBytesPerSec, double ulBytesPerSec);

signals:
  void graphScaleChanged();
  void phaseChanged();

protected:
  QSGNode *updatePaintNode(QSGNode *oldNode,
                           UpdatePaintNodeData *data) override;
  void releaseResources() override;

private:
  mutable HistoryTextureProvider *m_provider = nullptr;

  std::array<float, kSize> m_dlHistory{};
  std::array<float, kSize> m_ulHistory{};
  int m_writeIndex{};
  float m_graphMax{kMinMaxReference};
  float m_graphScale{1.0F};
  bool m_dirty{false};
  std::mutex m_mutex;
};
