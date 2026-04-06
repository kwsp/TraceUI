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

public:
    explicit TerminalRenderer(QQuickItem* parent = nullptr);
    ~TerminalRenderer() override;

    TerminalBackend* backend() const { return m_backend; }
    void setBackend(TerminalBackend* backend);

    QString fontData() const { return m_fontData; }
    void setFontData(const QString& font);

    int fontSize() const { return m_fontSize; }
    void setFontSize(int size);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

signals:
    void backendChanged();
    void fontDataChanged();
    void fontSizeChanged();

private:
    void updateGlyphAtlas();

    TerminalBackend* m_backend = nullptr;
    QString m_fontData = "Hack";
    int m_fontSize = 12;

    QRawFont m_rawFont;
    bool m_fontDirty = true;

    struct GlyphInfo {
        uint32_t index;
        QRectF uvRect;
    };
    QHash<uint32_t, GlyphInfo> m_glyphCache;
    std::unique_ptr<QSGTexture> m_atlasTexture;
};
