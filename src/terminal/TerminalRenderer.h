#pragma once

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
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

signals:
    void backendChanged();
    void fontFamilyChanged();
    void fontSizeChanged();
    void cellMetricsChanged();

private:
    void rebuildAtlas();
    void recalcMetrics();

    // ── Font / Atlas ─────────────────────────────────────────────────────────
    QString m_fontFamily{"Hack"};
    int m_fontSize{14};
    qreal m_cellWidth{};
    qreal m_cellHeight{};
    qreal m_ascent{};
    bool m_atlasDirty{true};

    QRawFont m_rawFont;

    // Codepoint → UV rect in atlas.  Keyed by Unicode codepoint (not glyph index).
    struct GlyphUV {
        float u1, v1, u2, v2;
    };
    QHash<uint32_t, GlyphUV> m_uvCache; // codepoint → UV
    GlyphUV m_spaceUV{};                // fallback for missing glyphs

    QImage m_atlasImage;          // kept for re-upload
    QSGTexture *m_atlasTexture{}; // owned by scene graph

    // ── Backend ──────────────────────────────────────────────────────────────
    TerminalBackend *m_backend{};
};
