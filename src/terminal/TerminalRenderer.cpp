#include "TerminalRenderer.h"
#include "GlyphAtlas.h"
#include "GlyphMaterial.h"
#include <QFontMetricsF>
#include <QPainter>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>
#include <cmath>
#include <span>
#include <vterm.h>

// NOLINTBEGIN(*-isolate-declaration, *-static-cast-downcast)

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
    if (m_backend != nullptr)
        disconnect(m_backend, nullptr, this, nullptr);
    m_backend = backend;
    if (m_backend != nullptr)
        connect(m_backend, &TerminalBackend::screenDamaged, this, [this] { update(); });
    emit backendChanged();
    update();
}

void TerminalRenderer::setFontFamily(const QString &family) {
    if (m_fontFamily == family)
        return;
    m_fontFamily = family;
    recalcMetrics();
    emit fontFamilyChanged();
    update();
}

void TerminalRenderer::setFontSize(int size) {
    if (size < 1 || m_fontSize == size)
        return;
    m_fontSize = size;
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

// ── Color helpers ────────────────────────────────────────────────────────────

static void vtermColorToRGBAf32(const VTermScreen *screen, VTermColor col, float &r, float &g,
                                float &b) {
    if (VTERM_COLOR_IS_INDEXED(&col))
        vterm_screen_convert_color_to_rgb(screen, &col);
    // NOLINTBEGIN(*-narrowing-conversions, *-magic-numbers)
    r = col.rgb.red / 255.0F;
    g = col.rgb.green / 255.0F;
    b = col.rgb.blue / 255.0F;
    // NOLINTEND(*-narrowing-conversions, *-magic-numbers)
}

static void vtermColorToRGBAu8(const VTermScreen *screen, VTermColor col, uint8_t &r, uint8_t &g,
                               uint8_t &b) {
    if (VTERM_COLOR_IS_INDEXED(&col))
        vterm_screen_convert_color_to_rgb(screen, &col);
    r = col.rgb.red;
    g = col.rgb.green;
    b = col.rgb.blue;
}

static const QSGGeometry::AttributeSet &glyphAttributeSet() {
    static QSGGeometry::Attribute data[] = {
        QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, true), // pos (x, y)
        QSGGeometry::Attribute::create(1, 2, QSGGeometry::FloatType),       // texcoord (u, v)
        QSGGeometry::Attribute::create(2, 4, QSGGeometry::FloatType),       // color (r, g, b, a)
    };
    static QSGGeometry::AttributeSet set = {
        .count = 3, .stride = sizeof(GlyphVertex), .attributes = data}; // NOLINT(*-pointer-decay)
    return set;
}

// ── Scene graph ──────────────────────────────────────────────────────────────
// Four-layer rendering:
//   1. Background node: colored quads for each cell's background
//   2. Glyph node: tinted textured quads for monospace text (alpha atlas, pre-baked)
//   3. Emoji node: full-color textured quads for emoji (RGBA atlas, passthrough shader)
//   4. Symbol node: tinted textured quads for NF SPUA-A icons (alpha atlas, lazy)

QSGNode *TerminalRenderer::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *_) {
    if ((m_backend == nullptr) || (window() == nullptr))
        return oldNode;

    const qreal dpr = (window() != nullptr) ? window()->devicePixelRatio() : 1.0;
    const QFontMetricsF fm(QFont(m_fontFamily, m_fontSize));
    m_emojiAtlas.setCellSize(m_cellWidth, m_cellHeight, dpr, fm.capHeight());
    m_glyphAtlas.setCellSize(m_cellWidth, m_cellHeight, dpr, m_ascent, m_fontFamily, m_fontSize);

    const int rows = m_backend->rows();
    const int cols = m_backend->cols();
    const int cellCount = rows * cols;
    constexpr int kVerticesPerCell = 6; // 2 triangles per cell
    const int vertexCount = cellCount * kVerticesPerCell;

    // ── Root node (container) ────────────────────────────────────────────
    // Structure: rootNode -> bgNode + glyphNode + emojiNode
    QSGNode *rootNode = oldNode;
    QSGGeometryNode *bgNode = nullptr;
    QSGGeometryNode *glyphNode = nullptr;
    QSGGeometryNode *emojiNode = nullptr;

    if (rootNode == nullptr) {
        rootNode = new QSGNode;

        // Background layer
        bgNode = [&]() {
            auto *bgNode = new QSGGeometryNode;
            bgNode->setFlag(QSGNode::OwnsGeometry);
            bgNode->setFlag(QSGNode::OwnsMaterial);
            auto *bgGeom =
                new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount);
            bgGeom->setDrawingMode(QSGGeometry::DrawTriangles);
            bgNode->setGeometry(bgGeom);
            auto *bgMat = new QSGVertexColorMaterial;
            bgNode->setMaterial(bgMat);
            return bgNode;
        }();
        rootNode->appendChildNode(bgNode);

        // Glyph layer
        glyphNode = [&]() {
            auto *glyphNode = new QSGGeometryNode;
            glyphNode->setFlag(QSGNode::OwnsGeometry);
            glyphNode->setFlag(QSGNode::OwnsMaterial);
            auto *glyphGeom = new QSGGeometry(glyphAttributeSet(), vertexCount);
            glyphGeom->setDrawingMode(QSGGeometry::DrawTriangles);
            glyphNode->setGeometry(glyphGeom);
            auto *glyphMat = new GlyphMaterial;
            glyphNode->setMaterial(glyphMat);
            return glyphNode;
        }();
        rootNode->appendChildNode(glyphNode);

        // Emoji layer
        emojiNode = [&]() {
            auto *node = new QSGGeometryNode;
            node->setFlag(QSGNode::OwnsGeometry);
            node->setFlag(QSGNode::OwnsMaterial);
            auto *geom = new QSGGeometry(glyphAttributeSet(), 0);
            geom->setDrawingMode(QSGGeometry::DrawTriangles);
            node->setGeometry(geom);
            auto *mat = new EmojiMaterial;
            node->setMaterial(mat);
            return node;
        }();
        rootNode->appendChildNode(emojiNode);
    } else {
        bgNode = static_cast<QSGGeometryNode *>(rootNode->childAtIndex(0));
        glyphNode = static_cast<QSGGeometryNode *>(rootNode->childAtIndex(1));
        emojiNode = static_cast<QSGGeometryNode *>(rootNode->childAtIndex(2));
    }

    // ── Resize geometry if needed ────────────────────────────────────────
    auto *bgGeom = bgNode->geometry();
    auto *glyphGeom = glyphNode->geometry();
    auto *emojiGeom = emojiNode->geometry();
    if (bgGeom->vertexCount() != vertexCount)
        bgGeom->allocate(vertexCount);
    // Pre-allocate to upper bound; trimmed after the cell loop
    glyphGeom->allocate(vertexCount);
    // Emoji vertices are collected into staging vectors
    QVarLengthArray<GlyphVertex, 256> emojiStagingBuf;

    auto *bgVerts = bgGeom->vertexDataAsColoredPoint2D();
    auto *glyphVerts = static_cast<GlyphVertex *>(glyphGeom->vertexData());

    std::span<QSGGeometry::ColoredPoint2D> bgVertSpan(bgVerts, vertexCount);
    std::span<GlyphVertex> glyphVertSpan(glyphVerts, vertexCount);

    const VTermScreen *screen = m_backend->screen();

    // ── Fill all layers ──────────────────────────────────────────────────
    int bvi = 0; // background vertex index
    int tvi = 0; // text vertex index
    const uint8_t bgA = 255;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const VTermPos pos = {r, c};
            VTermScreenCell cell;
            vterm_screen_get_cell(screen, pos, &cell);

            const uint32_t cp = cell.chars[0];

            float fgR, fgG, fgB;   // NOLINT(*-init-variables)
            uint8_t bgR, bgG, bgB; // NOLINT(*-init-variables)
            vtermColorToRGBAf32(screen, cell.fg, fgR, fgG, fgB);
            vtermColorToRGBAu8(screen, cell.bg, bgR, bgG, bgB);

            const auto x1 = static_cast<float>(c * m_cellWidth);
            const auto y1 = static_cast<float>(r * m_cellHeight);
            const auto x2 = static_cast<float>((c + 1) * m_cellWidth);
            const auto y2 = static_cast<float>((r + 1) * m_cellHeight);

            // NOLINTBEGIN(*-magic-numbers)
            // Background quad (all cells)
            bgVertSpan[bvi + 0].set(x1, y1, bgR, bgG, bgB, bgA);
            bgVertSpan[bvi + 1].set(x1, y2, bgR, bgG, bgB, bgA);
            bgVertSpan[bvi + 2].set(x2, y1, bgR, bgG, bgB, bgA);
            bgVertSpan[bvi + 3].set(x2, y1, bgR, bgG, bgB, bgA);
            bgVertSpan[bvi + 4].set(x1, y2, bgR, bgG, bgB, bgA);
            bgVertSpan[bvi + 5].set(x2, y2, bgR, bgG, bgB, bgA);
            bvi += kVerticesPerCell;

            // libvterm marks continuation cells of double-wide glyphs with chars[0] = UINT32_MAX.
            // Skip them here — the background quad was already written above.
            if (cp == static_cast<uint32_t>(-1))
                continue;

            const bool emojiGlyph = (cp != 0 && isEmoji(cp));

            if (emojiGlyph) {
                m_emojiAtlas.ensureGlyph(cp);
                const GlyphUV emojiUV = m_emojiAtlas.uv(cp);
                if (emojiUV.u2 > 0.0F) {
                    const auto ex2 = x1 + 2.0F * static_cast<float>(m_cellWidth);
                    GlyphVertex v[kVerticesPerCell];
                    v[0].set(x1, y1, emojiUV.u1, emojiUV.v1, 1.0F, 1.0F, 1.0F, 1.0F);
                    v[1].set(x1, y2, emojiUV.u1, emojiUV.v2, 1.0F, 1.0F, 1.0F, 1.0F);
                    v[2].set(ex2, y1, emojiUV.u2, emojiUV.v1, 1.0F, 1.0F, 1.0F, 1.0F);
                    v[3].set(ex2, y1, emojiUV.u2, emojiUV.v1, 1.0F, 1.0F, 1.0F, 1.0F);
                    v[4].set(x1, y2, emojiUV.u1, emojiUV.v2, 1.0F, 1.0F, 1.0F, 1.0F);
                    v[5].set(ex2, y2, emojiUV.u2, emojiUV.v2, 1.0F, 1.0F, 1.0F, 1.0F);
                    emojiStagingBuf.append(v, kVerticesPerCell);
                    continue;
                }
            }

            // Standard glyph or Nerd Font symbol
            m_glyphAtlas.ensureGlyph(cp, cell.width);
            const GlyphUV uv = m_glyphAtlas.uv(cp);
            const auto gx2 = (isWideIcon(cp) || cell.width == 2)
                                 ? static_cast<float>(x1 + 2.0F * static_cast<float>(m_cellWidth))
                                 : x2;

            glyphVertSpan[tvi + 0].set(x1, y1, uv.u1, uv.v1, fgR, fgG, fgB, 1.0F);
            glyphVertSpan[tvi + 1].set(x1, y2, uv.u1, uv.v2, fgR, fgG, fgB, 1.0F);
            glyphVertSpan[tvi + 2].set(gx2, y1, uv.u2, uv.v1, fgR, fgG, fgB, 1.0F);
            glyphVertSpan[tvi + 3].set(gx2, y1, uv.u2, uv.v1, fgR, fgG, fgB, 1.0F);
            glyphVertSpan[tvi + 4].set(x1, y2, uv.u1, uv.v2, fgR, fgG, fgB, 1.0F);
            glyphVertSpan[tvi + 5].set(gx2, y2, uv.u2, uv.v2, fgR, fgG, fgB, 1.0F);
            tvi += kVerticesPerCell;
            // NOLINTEND(*-magic-numbers)
        }
    }

    // Trim glyph geometry; upload emoji from staging buffer
    glyphGeom->allocate(tvi);
    const size_t evi = emojiStagingBuf.size();
    emojiGeom->allocate(static_cast<int>(evi));
    if (evi > 0)
        memcpy(emojiGeom->vertexData(), emojiStagingBuf.data(), evi * sizeof(GlyphVertex));

    // Re-upload glyph texture if atlas changed
    if (m_glyphAtlas.isDirty() || m_glyphTexture == nullptr) {
        delete m_glyphTexture;
        m_glyphTexture = window()->createTextureFromImage(m_glyphAtlas.image(),
                                                          QQuickWindow::TextureHasAlphaChannel);
        m_glyphTexture->setFiltering(QSGTexture::Linear);
        m_glyphAtlas.markClean();
        auto *glyphMat = static_cast<GlyphMaterial *>(glyphNode->material()); 
        glyphMat->setTexture(m_glyphTexture);
    }

    // Re-upload emoji texture if atlas changed
    if (m_emojiAtlas.isDirty() || m_emojiTexture == nullptr) {
        delete m_emojiTexture;
        m_emojiTexture = window()->createTextureFromImage(m_emojiAtlas.image(),
                                                          QQuickWindow::TextureHasAlphaChannel);
        m_emojiTexture->setFiltering(QSGTexture::Linear);
        m_emojiAtlas.markClean();
        auto *emojiMat = static_cast<EmojiMaterial *>(emojiNode->material());
        emojiMat->setTexture(m_emojiTexture);
    }

    bgNode->markDirty(QSGNode::DirtyGeometry);
    glyphNode->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
    emojiNode->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
    return rootNode;
}

// NOLINTEND(*-isolate-declaration, *-static-cast-downcast)