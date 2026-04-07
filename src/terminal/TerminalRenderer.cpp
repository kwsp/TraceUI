#include "TerminalRenderer.h"
#include "GlyphMaterial.h"
#include <QFontMetricsF>
#include <QPainter>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>
#include <cmath>
#include <vterm.h>

// ── Lifecycle ────────────────────────────────────────────────────────────────

TerminalRenderer::TerminalRenderer(QQuickItem *parent) : QQuickItem(parent) {
    setFlag(ItemHasContents, true);
    recalcMetrics();
}

TerminalRenderer::~TerminalRenderer() = default;

// ── Property setters ─────────────────────────────────────────────────────────

void TerminalRenderer::setBackend(TerminalBackend *backend) {
    if (m_backend == backend)
        return;
    if (m_backend)
        disconnect(m_backend, nullptr, this, nullptr);
    m_backend = backend;
    if (m_backend)
        connect(m_backend, &TerminalBackend::screenDamaged, this, [this] { update(); });
    emit backendChanged();
    update();
}

void TerminalRenderer::setFontFamily(const QString &family) {
    if (m_fontFamily == family)
        return;
    m_fontFamily = family;
    m_atlasDirty = true;
    recalcMetrics();
    emit fontFamilyChanged();
    update();
}

void TerminalRenderer::setFontSize(int size) {
    if (size < 1 || m_fontSize == size)
        return;
    m_fontSize = size;
    m_atlasDirty = true;
    recalcMetrics();
    emit fontSizeChanged();
    update();
}

void TerminalRenderer::ensureMetrics() {
    recalcMetrics();
}

void TerminalRenderer::geometryChange(const QRectF &newGeo, const QRectF &oldGeo) {
    QQuickItem::geometryChange(newGeo, oldGeo);
    update();
}

// ── Font metrics ─────────────────────────────────────────────────────────────

void TerminalRenderer::recalcMetrics() {
    QFont font(m_fontFamily, m_fontSize);
    QFontMetricsF fm(font);
    // Snap to integer cell dimensions to eliminate sub-pixel gaps between rows/cols.
    qreal w = std::ceil(fm.horizontalAdvance('M'));
    qreal h = std::ceil(fm.height());
    if (w != m_cellWidth || h != m_cellHeight) {
        m_cellWidth = w;
        m_cellHeight = h;
        m_ascent = fm.ascent();
        emit cellMetricsChanged();
    }
}

// ── Glyph atlas ──────────────────────────────────────────────────────────────
// Layout: 32-column grid covering ASCII + box-drawing + block elements.

struct CodepointRange {
    uint32_t first, last;
};

static constexpr CodepointRange kAtlasRanges[] = {
    {0x0020, 0x007E}, // ASCII printable (95)
    {0x00A0, 0x00FF}, // Latin-1 Supplement (96)
    {0x2500, 0x257F}, // Box Drawing (128)
    {0x2580, 0x259F}, // Block Elements (32)
    {0x2190, 0x21FF}, // Arrows (112)
    {0x25A0, 0x25FF}, // Geometric Shapes (96)
    {0x2600, 0x26FF}, // Misc Symbols (256)
};

static int totalGlyphCount() {
    int n = 0;
    for (const auto &r : kAtlasRanges)
        n += static_cast<int>(r.last - r.first + 1);
    return n;
}

void TerminalRenderer::rebuildAtlas() {
    if (!m_atlasDirty && !m_uvCache.isEmpty())
        return;

    QFont font(m_fontFamily, m_fontSize);

    const int glyphCount = totalGlyphCount();
    constexpr int kCols = 32;
    const int kRows = (glyphCount + kCols - 1) / kCols;

    qreal dpr = window() ? window()->devicePixelRatio() : 1.0;
    qreal margin = 1.0; // 1 logical pixel padding to prevent antialiasing bleed

    qreal cellBoxW = m_cellWidth + 2.0 * margin;
    qreal cellBoxH = m_cellHeight + 2.0 * margin;

    int atlasPhysW = std::ceil(kCols * cellBoxW * dpr);
    int atlasPhysH = std::ceil(kRows * cellBoxH * dpr);

    m_atlasImage = QImage(atlasPhysW, atlasPhysH, QImage::Format_ARGB32_Premultiplied);
    m_atlasImage.setDevicePixelRatio(dpr);
    m_atlasImage.fill(Qt::transparent);

    QPainter painter(&m_atlasImage);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    m_uvCache.clear();
    m_uvCache.reserve(glyphCount);

    qreal atlasLogicalW = kCols * cellBoxW;
    qreal atlasLogicalH = kRows * cellBoxH;

    int idx = 0;
    for (const auto &range : kAtlasRanges) {
        for (uint32_t cp = range.first; cp <= range.last; ++cp, ++idx) {
            int col = idx % kCols;
            int row = idx / kCols;

            qreal logicalX = col * cellBoxW;
            qreal logicalY = row * cellBoxH;

            // Draw text inside the margin
            painter.drawText(QPointF(logicalX + margin, logicalY + margin + m_ascent),
                             QString::fromUcs4(&cp, 1));

            GlyphUV uv;
            qreal u1 = logicalX + margin;
            qreal v1 = logicalY + margin;
            qreal u2 = u1 + m_cellWidth;
            qreal v2 = v1 + m_cellHeight;

            uv.u1 = static_cast<float>(u1 / atlasLogicalW);
            uv.v1 = static_cast<float>(v1 / atlasLogicalH);
            uv.u2 = static_cast<float>(u2 / atlasLogicalW);
            uv.v2 = static_cast<float>(v2 / atlasLogicalH);

            m_uvCache[cp] = uv;
        }
    }

    painter.end();
    m_spaceUV = m_uvCache.value(' ');
    m_atlasDirty = false;
}

// ── Color helpers ────────────────────────────────────────────────────────────

static void vtermColorToRGBA(const VTermScreen *screen, VTermColor col, float &r, float &g,
                             float &b) {
    if (VTERM_COLOR_IS_INDEXED(&col))
        vterm_screen_convert_color_to_rgb(screen, &col);
    r = col.rgb.red / 255.0F;
    g = col.rgb.green / 255.0F;
    b = col.rgb.blue / 255.0F;
}

static const QSGGeometry::AttributeSet &glyphAttributeSet() {
    static QSGGeometry::Attribute data[] = {
        QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, true), // pos
        QSGGeometry::Attribute::create(1, 2, QSGGeometry::FloatType),       // texcoord
        QSGGeometry::Attribute::create(2, 4, QSGGeometry::FloatType),       // color
    };
    static QSGGeometry::AttributeSet set = {3, sizeof(GlyphVertex), data};
    return set;
}

// ── Scene graph ──────────────────────────────────────────────────────────────
// Two-layer rendering:
//   1. Background node: colored quads for each cell's background
//   2. Glyph node: textured quads (atlas) with QSGTextureMaterial (proven working)

QSGNode *TerminalRenderer::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    if (!m_backend || !window())
        return oldNode;

    rebuildAtlas();

    // (Re-)create atlas texture if needed
    if (!m_atlasTexture || m_atlasTexture->textureSize() != m_atlasImage.size()) {
        delete m_atlasTexture;
        m_atlasTexture =
            window()->createTextureFromImage(m_atlasImage, QQuickWindow::TextureHasAlphaChannel);
        // Linear filtering handles non-integer Retina scaling gracefully
        m_atlasTexture->setFiltering(QSGTexture::Linear);
    }

    const int rows = m_backend->rows();
    const int cols = m_backend->cols();
    const int cellCount = rows * cols;
    const int vertexCount = cellCount * 6;

    // ── Root node (container) ────────────────────────────────────────────
    // Structure: rootNode -> bgNode + glyphNode
    QSGNode *rootNode = oldNode;
    QSGGeometryNode *bgNode = nullptr;
    QSGGeometryNode *glyphNode = nullptr;

    if (!rootNode) {
        rootNode = new QSGNode;

        // Background layer
        bgNode = new QSGGeometryNode;
        bgNode->setFlag(QSGNode::OwnsGeometry);
        bgNode->setFlag(QSGNode::OwnsMaterial);
        auto *bgGeom =
            new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount);
        bgGeom->setDrawingMode(QSGGeometry::DrawTriangles);
        bgNode->setGeometry(bgGeom);
        auto *bgMat = new QSGVertexColorMaterial;
        bgNode->setMaterial(bgMat);
        rootNode->appendChildNode(bgNode);

        // Glyph layer
        glyphNode = new QSGGeometryNode;
        glyphNode->setFlag(QSGNode::OwnsGeometry);
        glyphNode->setFlag(QSGNode::OwnsMaterial);
        auto *glyphGeom = new QSGGeometry(glyphAttributeSet(), vertexCount);
        glyphGeom->setDrawingMode(QSGGeometry::DrawTriangles);
        glyphNode->setGeometry(glyphGeom);
        auto *glyphMat = new GlyphMaterial;
        glyphMat->setTexture(m_atlasTexture);
        glyphNode->setMaterial(glyphMat);
        rootNode->appendChildNode(glyphNode);
    } else {
        bgNode = static_cast<QSGGeometryNode *>(rootNode->childAtIndex(0));
        glyphNode = static_cast<QSGGeometryNode *>(rootNode->childAtIndex(1));

        auto *glyphMat = static_cast<GlyphMaterial *>(glyphNode->material());
        glyphMat->setTexture(m_atlasTexture);
    }

    // ── Resize geometry if needed ────────────────────────────────────────
    auto *bgGeom = bgNode->geometry();
    auto *glyphGeom = glyphNode->geometry();
    if (bgGeom->vertexCount() != vertexCount)
        bgGeom->allocate(vertexCount);
    if (glyphGeom->vertexCount() != vertexCount)
        glyphGeom->allocate(vertexCount);

    auto *bgVerts = bgGeom->vertexDataAsColoredPoint2D();
    auto *glyphVerts = static_cast<GlyphVertex *>(glyphGeom->vertexData());
    VTermScreen *screen = m_backend->screen();

    // ── Fill both layers ─────────────────────────────────────────────────
    int vi = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            VTermPos pos = {r, c};
            VTermScreenCell cell;
            vterm_screen_get_cell(screen, pos, &cell);

            // Character
            uint32_t cp = cell.chars[0];
            if (cp == 0)
                cp = ' ';
            const GlyphUV &uv = m_uvCache.value(cp, m_spaceUV);

            // Colors
            float fgR, fgG, fgB, bgR, bgG, bgB;
            vtermColorToRGBA(screen, cell.fg, fgR, fgG, fgB);
            vtermColorToRGBA(screen, cell.bg, bgR, bgG, bgB);

            // Cell rect
            float x1 = static_cast<float>(c * m_cellWidth);
            float y1 = static_cast<float>(r * m_cellHeight);
            float x2 = static_cast<float>((c + 1) * m_cellWidth);
            float y2 = static_cast<float>((r + 1) * m_cellHeight);

            // BG color as uchar RGBA
            auto bgRi = static_cast<uchar>(bgR * 255);
            auto bgGi = static_cast<uchar>(bgG * 255);
            auto bgBi = static_cast<uchar>(bgB * 255);

            // Background triangles (solid color)
            bgVerts[vi + 0].set(x1, y1, bgRi, bgGi, bgBi, 255);
            bgVerts[vi + 1].set(x1, y2, bgRi, bgGi, bgBi, 255);
            bgVerts[vi + 2].set(x2, y1, bgRi, bgGi, bgBi, 255);
            bgVerts[vi + 3].set(x2, y1, bgRi, bgGi, bgBi, 255);
            bgVerts[vi + 4].set(x1, y2, bgRi, bgGi, bgBi, 255);
            bgVerts[vi + 5].set(x2, y2, bgRi, bgGi, bgBi, 255);

            // Glyph triangles (textured + colored)
            glyphVerts[vi + 0].set(x1, y1, uv.u1, uv.v1, fgR, fgG, fgB, 1.0f);
            glyphVerts[vi + 1].set(x1, y2, uv.u1, uv.v2, fgR, fgG, fgB, 1.0f);
            glyphVerts[vi + 2].set(x2, y1, uv.u2, uv.v1, fgR, fgG, fgB, 1.0f);
            glyphVerts[vi + 3].set(x2, y1, uv.u2, uv.v1, fgR, fgG, fgB, 1.0f);
            glyphVerts[vi + 4].set(x1, y2, uv.u1, uv.v2, fgR, fgG, fgB, 1.0f);
            glyphVerts[vi + 5].set(x2, y2, uv.u2, uv.v2, fgR, fgG, fgB, 1.0f);

            vi += 6;

            if (cell.width > 1)
                c += cell.width - 1;
        }
    }

    bgNode->markDirty(QSGNode::DirtyGeometry);
    glyphNode->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
    return rootNode;
}
