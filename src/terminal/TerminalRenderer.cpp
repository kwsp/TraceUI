#include "TerminalRenderer.h"
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QQuickWindow>
#include <QSGFlatColorMaterial>
#include <QSGGeometryNode>
#include <QSGTextureMaterial>

TerminalRenderer::TerminalRenderer(QQuickItem* parent) : QQuickItem(parent) {
    setFlag(ItemHasContents, true);
}

TerminalRenderer::~TerminalRenderer() = default;

void TerminalRenderer::setBackend(TerminalBackend* backend) {
    if (m_backend == backend) return;
    if (m_backend) {
        disconnect(m_backend, &TerminalBackend::screenDamaged, this, &TerminalRenderer::update);
    }
    m_backend = backend;
    if (m_backend) {
        connect(m_backend, &TerminalBackend::screenDamaged, this, &TerminalRenderer::update);
    }
    emit backendChanged();
    update();
}

void TerminalRenderer::setFontData(const QString& font) {
    if (m_fontData == font) return;
    m_fontData = font;
    m_fontDirty = true;
    emit fontDataChanged();
    update();
}

void TerminalRenderer::setFontSize(int size) {
    if (m_fontSize == size) return;
    m_fontSize = size;
    m_fontDirty = true;
    emit fontSizeChanged();
    update();
}

void TerminalRenderer::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) {
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    update();
}

void TerminalRenderer::updateGlyphAtlas() {
    if (!m_fontDirty && m_atlasTexture) return;

    QFont font(m_fontData, m_fontSize);
    m_rawFont = QRawFont::fromFont(font);
    m_glyphCache.clear();

    // Basic ASCII for now
    QList<quint32> glyphIndices;
    for (int i = 32; i < 127; ++i) {
        glyphIndices.append(m_rawFont.glyphIndexesForString(QString(QChar(i)))[0]);
    }

    // Determine cell size
    QFontMetricsF fm(font);
    qreal cellWidth = fm.horizontalAdvance('W');
    qreal cellHeight = fm.height();

    // Create atlas (simple horizontal strip for now)
    int atlasWidth = static_cast<int>(std::ceil(cellWidth * glyphIndices.size()));
    int atlasHeight = static_cast<int>(std::ceil(cellHeight));

    QImage atlas(atlasWidth, atlasHeight, QImage::Format_Alpha8);
    atlas.fill(0);
    QPainter painter(&atlas);
    painter.setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < glyphIndices.size(); ++i) {
        uint32_t index = glyphIndices[i];
        QPointF pos(i * cellWidth, m_rawFont.ascent());
        QPainterPath path = m_rawFont.pathForGlyph(index);
        painter.fillPath(path.translated(pos), Qt::white);

        m_glyphCache[index] = {index, QRectF(qreal(i * cellWidth) / atlasWidth, 0,
                                             cellWidth / atlasWidth, 1.0)};
    }

    m_atlasTexture.reset(window()->createTextureFromImage(atlas));
    m_fontDirty = false;
}

QSGNode* TerminalRenderer::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) {
    if (!m_backend || !window()) return oldNode;

    updateGlyphAtlas();

    auto* node = static_cast<QSGGeometryNode*>(oldNode);
    if (!node) {
        node = new QSGGeometryNode;
        node->setFlag(QSGNode::OwnsGeometry);
        node->setFlag(QSGNode::OwnsMaterial);

        auto* material = new QSGTextureMaterial;
        material->setTexture(m_atlasTexture.get());
        node->setMaterial(material);

        auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 0);
        node->setGeometry(geometry);
    }

    // Just render a placeholder quad using the atlas for now to verify it works
    auto* geometry = node->geometry();
    geometry->allocate(4);
    auto* vertices = geometry->vertexDataAsTexturedPoint2D();
    const QRectF rect = boundingRect();

    vertices[0].set(rect.left(), rect.top(), 0, 0);
    vertices[1].set(rect.left(), rect.bottom(), 0, 1);
    vertices[2].set(rect.right(), rect.top(), 1, 0);
    vertices[3].set(rect.right(), rect.bottom(), 1, 1);

    node->markDirty(QSGNode::DirtyGeometry);
    return node;
}
