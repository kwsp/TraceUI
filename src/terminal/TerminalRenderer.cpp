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
    m_metricsDirty = true;
    emit fontDataChanged();
    update();
}

void TerminalRenderer::setFontSize(int size) {
    if (m_fontSize == size) return;
    m_fontSize = size;
    m_fontDirty = true;
    m_metricsDirty = true;
    emit fontSizeChanged();
    update();
}

void TerminalRenderer::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) {
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    update();
}

void TerminalRenderer::updateCellMetrics() {
    if (!m_metricsDirty) return;

    QFont font(m_fontData, m_fontSize);
    QFontMetricsF fm(font);
    m_cellWidth = fm.horizontalAdvance('W');
    m_cellHeight = fm.height();
    m_metricsDirty = false;
    emit cellMetricsChanged();
}

void TerminalRenderer::updateGlyphAtlas() {
    updateCellMetrics();
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
    qreal cellWidth = m_cellWidth;
    qreal cellHeight = m_cellHeight;

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

    int rows = m_backend->rows();
    int cols = m_backend->cols();
    
    // Each cell is 2 triangles (6 vertices)
    int vertexCount = rows * cols * 6;
    auto* geometry = node->geometry();
    geometry->allocate(vertexCount);
    auto* vertices = geometry->vertexDataAsTexturedPoint2D();

    int vIdx = 0;
    for (int r = 0; r < rows; ++r) {
        QString line = m_backend->getLineText(r);
        for (int c = 0; c < cols; ++c) {
            uint32_t cp = (c < line.length()) ? line[c].unicode() : ' ';
            
            // Simplified: only ASCII 32-126 supported in atlas for now
            if (cp < 32 || cp > 126) cp = ' ';
            
            uint32_t glyphIdx = m_rawFont.glyphIndexesForString(QString(QChar(cp)))[0];
            const auto& gInfo = m_glyphCache[glyphIdx];

            qreal x1 = c * m_cellWidth;
            qreal y1 = r * m_cellHeight;
            qreal x2 = x1 + m_cellWidth;
            qreal y2 = y1 + m_cellHeight;

            float u1 = gInfo.uvRect.left();
            float v1 = gInfo.uvRect.top();
            float u2 = gInfo.uvRect.right();
            float v2 = gInfo.uvRect.bottom();

            // Triangle 1
            vertices[vIdx++].set(x1, y1, u1, v1);
            vertices[vIdx++].set(x1, y2, u1, v2);
            vertices[vIdx++].set(x2, y1, u2, v1);
            // Triangle 2
            vertices[vIdx++].set(x2, y1, u2, v1);
            vertices[vIdx++].set(x1, y2, u1, v2);
            vertices[vIdx++].set(x2, y2, u2, v2);
        }
    }

    node->markDirty(QSGNode::DirtyGeometry);
    return node;
}
