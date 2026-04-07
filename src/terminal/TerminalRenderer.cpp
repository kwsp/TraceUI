#include "TerminalRenderer.h"
#include "TerminalMaterial.h"
#include <QFontMetricsF>
#include <QPainter>
#include <QQuickWindow>
#include <QSGGeometryNode>
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
// Layout: 16-column grid covering ASCII + box-drawing + block elements.

// Codepoint ranges to include in the atlas.
struct CodepointRange {
    uint32_t first, last;
};

static constexpr CodepointRange kAtlasRanges[] = {
    {0x0020, 0x007E}, // ASCII printable (95)
    {0x00A0, 0x00FF}, // Latin-1 Supplement (96) — accented chars, ¡¢£ etc.
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

    int cw = static_cast<int>(m_cellWidth);
    int ch = static_cast<int>(m_cellHeight);
    int atlasW = cw * kCols;
    int atlasH = ch * kRows;

    m_atlasImage = QImage(atlasW, atlasH, QImage::Format_ARGB32_Premultiplied);
    m_atlasImage.fill(Qt::transparent);

    QPainter painter(&m_atlasImage);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.setRenderHint(QPainter::Antialiasing, true);

    m_uvCache.clear();
    m_uvCache.reserve(glyphCount);

    int idx = 0;
    for (const auto &range : kAtlasRanges) {
        for (uint32_t cp = range.first; cp <= range.last; ++cp, ++idx) {
            int col = idx % kCols;
            int row = idx / kCols;
            qreal x = col * m_cellWidth;
            qreal y = row * m_cellHeight + m_ascent;

            painter.drawText(QPointF(x, y), QString::fromUcs4(&cp, 1));

            GlyphUV uv;
            uv.u1 = static_cast<float>(col * m_cellWidth) / atlasW;
            uv.v1 = static_cast<float>(row * m_cellHeight) / atlasH;
            uv.u2 = static_cast<float>((col + 1) * m_cellWidth) / atlasW;
            uv.v2 = static_cast<float>((row + 1) * m_cellHeight) / atlasH;
            m_uvCache[cp] = uv;
        }
    }

    painter.end();
    m_spaceUV = m_uvCache.value(' ');
    m_atlasDirty = false;
}

// ── Color helpers ────────────────────────────────────────────────────────────

static void vtermColorToFloat(const VTermScreen *screen, VTermColor col, float &r, float &g,
                              float &b) {
    if (VTERM_COLOR_IS_INDEXED(&col))
        vterm_screen_convert_color_to_rgb(screen, &col);
    r = col.rgb.red / 255.0F;
    g = col.rgb.green / 255.0F;
    b = col.rgb.blue / 255.0F;
}

// ── Scene graph ──────────────────────────────────────────────────────────────

QSGNode *TerminalRenderer::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    if (!m_backend || !window())
        return oldNode;

    rebuildAtlas();

    // (Re-)create atlas texture if needed
    if (!m_atlasTexture || m_atlasTexture->textureSize() != m_atlasImage.size()) {
        delete m_atlasTexture;
        m_atlasTexture =
            window()->createTextureFromImage(m_atlasImage, QQuickWindow::TextureCanUseAtlas);
    }

    const int rows = m_backend->rows();
    const int cols = m_backend->cols();
    const int cellCount = rows * cols;
    const int vertexCount = cellCount * 6; // 2 triangles per cell

    auto *node = static_cast<QSGGeometryNode *>(oldNode);
    if (!node) {
        node = new QSGGeometryNode;
        node->setFlag(QSGNode::OwnsGeometry);
        node->setFlag(QSGNode::OwnsMaterial);

        auto *geom = new QSGGeometry(terminalAttributeSet(), vertexCount);
        geom->setDrawingMode(QSGGeometry::DrawTriangles);
        node->setGeometry(geom);

        auto *mat = new TerminalMaterial;
        mat->setTexture(m_atlasTexture);
        node->setMaterial(mat);
    } else {
        auto *mat = static_cast<TerminalMaterial *>(node->material());
        mat->setTexture(m_atlasTexture);
    }

    auto *geom = node->geometry();
    if (geom->vertexCount() != vertexCount)
        geom->allocate(vertexCount);

    auto *verts = static_cast<TerminalVertex *>(geom->vertexData());
    VTermScreen *screen = m_backend->screen();

    int vi = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            VTermPos pos = {r, c};
            VTermScreenCell cell;
            vterm_screen_get_cell(screen, pos, &cell);

            uint32_t cp = cell.chars[0];
            if (cp == 0)
                cp = ' ';

            const GlyphUV &uv = m_uvCache.value(cp, m_spaceUV);

            float fgR, fgG, fgB, bgR, bgG, bgB;
            vtermColorToFloat(screen, cell.fg, fgR, fgG, fgB);
            vtermColorToFloat(screen, cell.bg, bgR, bgG, bgB);

            float x1 = static_cast<float>(c * m_cellWidth);
            float y1 = static_cast<float>(r * m_cellHeight);
            float x2 = static_cast<float>((c + 1) * m_cellWidth);
            float y2 = static_cast<float>((r + 1) * m_cellHeight);

            // Two triangles per cell
            verts[vi++].set(x1, y1, uv.u1, uv.v1, fgR, fgG, fgB, 1, bgR, bgG, bgB, 1);
            verts[vi++].set(x1, y2, uv.u1, uv.v2, fgR, fgG, fgB, 1, bgR, bgG, bgB, 1);
            verts[vi++].set(x2, y1, uv.u2, uv.v1, fgR, fgG, fgB, 1, bgR, bgG, bgB, 1);

            verts[vi++].set(x2, y1, uv.u2, uv.v1, fgR, fgG, fgB, 1, bgR, bgG, bgB, 1);
            verts[vi++].set(x1, y2, uv.u1, uv.v2, fgR, fgG, fgB, 1, bgR, bgG, bgB, 1);
            verts[vi++].set(x2, y2, uv.u2, uv.v2, fgR, fgG, fgB, 1, bgR, bgG, bgB, 1);

            if (cell.width > 1)
                c += cell.width - 1;
        }
    }

    node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
    return node;
}
