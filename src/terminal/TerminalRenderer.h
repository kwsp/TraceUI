#pragma once

#include <QQuickItem>
#include <QSGGeometryNode>
#include <QSGTexture>
#include <QRawFont>
#include <QHash>
#include "TerminalBackend.h"

class TerminalRenderer : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(TerminalBackend* backend READ backend WRITE setBackend NOTIFY backendChanged)
    Q_PROPERTY(QString fontData READ fontData WRITE setFontData NOTIFY fontDataChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(qreal cellWidth READ cellWidth NOTIFY cellMetricsChanged)
    Q_PROPERTY(qreal cellHeight READ cellHeight NOTIFY cellMetricsChanged)

public:
    explicit TerminalRenderer(QQuickItem* parent = nullptr);
    ~TerminalRenderer() override;

    TerminalBackend* backend() const { return m_backend; }
    void setBackend(TerminalBackend* backend);

    QString fontData() const { return m_fontData; }
    void setFontData(const QString& font);

    int fontSize() const { return m_fontSize; }
    void setFontSize(int size);

    qreal cellWidth() const { 
        const_cast<TerminalRenderer*>(this)->updateCellMetrics();
        return m_cellWidth; 
    }
    qreal cellHeight() const { 
        const_cast<TerminalRenderer*>(this)->updateCellMetrics();
        return m_cellHeight; 
    }

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

signals:
    void backendChanged();
    void fontDataChanged();
    void fontSizeChanged();
    void cellMetricsChanged();

private:
    void updateGlyphAtlas();
    void updateCellMetrics();

    TerminalBackend* m_backend = nullptr;
    QString m_fontData = "Hack";
    int m_fontSize = 12;
    qreal m_cellWidth = 0;
    qreal m_cellHeight = 0;

    QRawFont m_rawFont;
    bool m_fontDirty = true;
    bool m_metricsDirty = true;

    struct GlyphInfo {
        uint32_t index;
        QRectF uvRect;
    };
    QHash<uint32_t, GlyphInfo> m_glyphCache;
    std::unique_ptr<QSGTexture> m_atlasTexture;

    int m_rows = 0;
    int m_cols = 0;
};
