#pragma once

#include "EmojiAtlas.h"
#include "EmojiMaterial.h"
#include "GlyphAtlas.h"
#include "GlyphMaterial.h"
#include "TerminalBackend.h"
#include <QHash>
#include <QQuickItem>
#include <QRawFont>
#include <QSGGeometryNode>
#include <QSGTexture>

class TerminalRenderer : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(TerminalBackend *backend READ backend WRITE setBackend NOTIFY backendChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(qreal cellWidth READ cellWidth NOTIFY cellMetricsChanged)
    Q_PROPERTY(qreal cellHeight READ cellHeight NOTIFY cellMetricsChanged)

public:
    explicit TerminalRenderer(QQuickItem *parent = nullptr);
    ~TerminalRenderer() override;

    TerminalRenderer(const TerminalRenderer &) = delete;
    TerminalRenderer(TerminalRenderer &&) = delete;
    TerminalRenderer &operator=(const TerminalRenderer &) = delete;
    TerminalRenderer &operator=(TerminalRenderer &&) = delete;

    TerminalBackend *backend() const { return m_backend; }
    void setBackend(TerminalBackend *backend);

    QString fontFamily() const { return m_fontFamily; }
    void setFontFamily(const QString &family);

    int fontSize() const { return m_fontSize; }
    void setFontSize(int size);

    qreal cellWidth() const { return m_cellWidth; }
    qreal cellHeight() const { return m_cellHeight; }

    // Force metrics recalculation (e.g. after construction in tests)
    void ensureMetrics();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *_) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

signals:
    void backendChanged();
    void fontFamilyChanged();
    void fontSizeChanged();
    void cellMetricsChanged();

private:
    void recalcMetrics();

    // ── Font / Metrics ───────────────────────────────────────────────────────
    QString m_fontFamily{"Hack"};
    int m_fontSize{14};
    qreal m_cellWidth{};
    qreal m_cellHeight{};
    qreal m_ascent{};

    // ── Glyph Atlas (Pre-rendered ASCII + Lazy Symbols) ──────────────────────
    GlyphAtlas m_glyphAtlas;
    QSGTexture *m_glyphTexture{}; // owned by scene graph

    // ── Emoji Atlas ──────────────────────────────────────────────────────────
    EmojiAtlas m_emojiAtlas;
    QSGTexture *m_emojiTexture{}; // owned by scene graph

    // ── Backend ──────────────────────────────────────────────────────────────
    TerminalBackend *m_backend{};
};
