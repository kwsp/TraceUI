#include "TerminalRenderer.h"
#include "GlyphMaterial.h"
#include <QFontMetricsF>
#include <QPainter>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>
#include <cmath>
#include <span>
#include <vterm.h>

// NOLINTBEGIN(*-isolate-declaration)

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
    char32_t first, last;
    bool wide = false; // true → 2-cell-wide slot in the atlas
};

// NOLINTBEGIN(*-designated-initializers)
static constexpr CodepointRange kAtlasRanges[] = {
    {0x0020, 0x007E}, // ASCII printable (95)
    {0x00A0, 0x00FF}, // Latin-1 Supplement (96)
    {0x2500, 0x257F}, // Box Drawing (128)
    {0x2580, 0x259F}, // Block Elements (32)
    {0x2190, 0x21FF}, // Arrows (112)
    {0x25A0, 0x25FF}, // Geometric Shapes (96)
    // {0x2600, 0x26FF} removed — handled by emoji atlas
    // ── Nerd Font v3 BMP ranges (SymbolsNerdFontMono) — 2-cell-wide ────────
    {0xE000, 0xE00A, true}, // Custom / Seti-UI
    {0xE0A0, 0xE0D7, true}, // Powerline + Powerline Extra
    {0xE200, 0xE2A9, true}, // IEC Power Symbols
    {0xE300, 0xE3E3, true}, // Font Logos (formerly Linux Logo)
    {0xE5FA, 0xE6B7, true}, // Seti-UI Extra
    {0xE700, 0xE8EF, true}, // Devicons
    {0xEA60, 0xEC1E, true}, // Codicons
    {0xED00, 0xEFCE, true}, // Material Design icons
    {0xF000, 0xF533, true}, // Font Awesome + Octicons + Powerline Extra
};
// NOLINTEND(*-designated-initializers)

static int totalGlyphCount() {
    int n = 0;
    for (const auto &r : kAtlasRanges)
        n += static_cast<int>(r.last - r.first + 1);
    return n;
}

// Column slots consumed in the atlas grid (wide glyphs occupy 2 columns each).
static int totalAtlasSlots() {
    int n = 0;
    for (const auto &r : kAtlasRanges)
        n += static_cast<int>(r.last - r.first + 1) * (r.wide ? 2 : 1);
    return n;
}

// Returns true for codepoints whose atlas slot is 2-cell wide (Nerd Font ranges).
// Used to override cell.width, which libvterm always reports as 1 for PUA codepoints.
static bool isWideAtlasGlyph(uint32_t cp) {
    for (const auto &r : kAtlasRanges)
        if (r.wide && cp >= r.first && cp <= r.last)
            return true;
    return false;
}

void TerminalRenderer::rebuildAtlas() {
    if (!m_atlasDirty && !m_uvCache.isEmpty())
        return;

    QFont font(m_fontFamily, m_fontSize);
    font.setFamilies({m_fontFamily, QStringLiteral("Symbols Nerd Font Mono")});

    const int glyphCount = totalGlyphCount();
    // +1 row safety for any row-wrap padding introduced by wide glyphs.
    constexpr int kCols = 32;
    const int kRows = (totalAtlasSlots() + kCols - 1) / kCols + 1;

    const qreal dpr = (window() != nullptr) ? window()->devicePixelRatio() : 1.0;
    const qreal margin = 1.0; // 1 logical pixel padding to prevent antialiasing bleed

    const qreal cellBoxW = m_cellWidth + margin + margin;
    const qreal cellBoxH = m_cellHeight + margin + margin;

    const int atlasPhysW = std::ceil(kCols * cellBoxW * dpr);
    const int atlasPhysH = std::ceil(kRows * cellBoxH * dpr);

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

    const qreal atlasLogicalW = kCols * cellBoxW;
    const qreal atlasLogicalH = kRows * cellBoxH;

    // idx is a column-slot cursor. Wide glyphs consume 2 columns; if a wide glyph
    // would start on the last column of a row it wraps to the next row instead.
    int idx = 0;
    for (const auto &range : kAtlasRanges) {
        for (char32_t cp = range.first; cp <= range.last; ++cp) {
            if (range.wide && (idx % kCols) == kCols - 1)
                ++idx; // skip lone trailing column — wrap wide glyph to next row

            const int col = idx % kCols;
            const int row = idx / kCols;

            const qreal logicalX = col * cellBoxW;
            const qreal logicalY = row * cellBoxH;
            const qreal glyphW   = range.wide ? 2.0 * m_cellWidth : m_cellWidth;

            // Draw text inside the margin
            painter.drawText(QPointF(logicalX + margin, logicalY + margin + m_ascent),
                             QString(QChar(cp)));

            const qreal u1 = logicalX + margin;
            const qreal v1 = logicalY + margin;
            const qreal u2 = u1 + glyphW;
            const qreal v2 = v1 + m_cellHeight;

            GlyphUV uv{
                .u1 = static_cast<float>(u1 / atlasLogicalW),
                .v1 = static_cast<float>(v1 / atlasLogicalH),
                .u2 = static_cast<float>(u2 / atlasLogicalW),
                .v2 = static_cast<float>(v2 / atlasLogicalH),
            };

            m_uvCache[cp] = uv;
            idx += range.wide ? 2 : 1;
        }
    }

    painter.end();
    m_spaceUV = m_uvCache.value(' ');
    m_atlasDirty = false;
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
// Three-layer rendering:
//   1. Background node: colored quads for each cell's background
//   2. Glyph node: tinted textured quads for monospace text (alpha atlas)
//   3. Emoji node: full-color textured quads for emoji (RGBA atlas, passthrough shader)

QSGNode *TerminalRenderer::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *_) {
    if ((m_backend == nullptr) || (window() == nullptr))
        return oldNode;

    rebuildAtlas();

    const qreal dpr = (window() != nullptr) ? window()->devicePixelRatio() : 1.0;
    const QFontMetricsF fm(QFont(m_fontFamily, m_fontSize));
    m_emojiAtlas.setCellSize(m_cellWidth, m_cellHeight, dpr, fm.capHeight());

    // (Re-)create atlas texture if needed
    if ((m_atlasTexture == nullptr) || m_atlasTexture->textureSize() != m_atlasImage.size()) {
        delete m_atlasTexture;
        m_atlasTexture =
            window()->createTextureFromImage(m_atlasImage, QQuickWindow::TextureHasAlphaChannel);
        // Linear filtering handles non-integer Retina scaling gracefully
        m_atlasTexture->setFiltering(QSGTexture::Linear);
    }

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
            glyphMat->setTexture(m_atlasTexture);
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
        // NOLINTBEGIN(*-static-cast-downcast)
        bgNode = static_cast<QSGGeometryNode *>(rootNode->childAtIndex(0));

        glyphNode = static_cast<QSGGeometryNode *>(rootNode->childAtIndex(1));
        auto *glyphMat = static_cast<GlyphMaterial *>(glyphNode->material());
        glyphMat->setTexture(m_atlasTexture);

        emojiNode = static_cast<QSGGeometryNode *>(rootNode->childAtIndex(2));
        auto *emojiMat = static_cast<EmojiMaterial *>(emojiNode->material());
        emojiMat->setTexture(m_emojiTexture);
        // NOLINTEND(*-static-cast-downcast)
    }

    // ── Resize geometry if needed ────────────────────────────────────────
    auto *bgGeom = bgNode->geometry();
    auto *glyphGeom = glyphNode->geometry();
    auto *emojiGeom = emojiNode->geometry();
    if (bgGeom->vertexCount() != vertexCount)
        bgGeom->allocate(vertexCount);
    // Pre-allocate to upper bound; trimmed after the cell loop
    glyphGeom->allocate(vertexCount);
    // Emoji vertices are collected into a staging vector (allocate() zeros
    // its buffer, so we cannot write then trim in-place).
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

            if (cp != 0 && isEmoji(cp))
                m_emojiAtlas.ensureGlyph(cp); // no-op on cache hit; sets dirty on first insert

            const GlyphUV emojiUV = (cp != 0 && isEmoji(cp)) ? m_emojiAtlas.uv(cp) : GlyphUV{};
            if (emojiUV.u2 > 0.0F) { // u2==0 means not in atlas (full or PUA fallback)
                const GlyphUV &uv = emojiUV;
                // 2-wide quad pushed into staging buffer
                const auto ex2 = static_cast<float>(x1 + 2.0F * m_cellWidth);
                GlyphVertex v[kVerticesPerCell];
                v[0].set(x1, y1, uv.u1, uv.v1, 1.0F, 1.0F, 1.0F, 1.0F);
                v[1].set(x1, y2, uv.u1, uv.v2, 1.0F, 1.0F, 1.0F, 1.0F);
                v[2].set(ex2, y1, uv.u2, uv.v1, 1.0F, 1.0F, 1.0F, 1.0F);
                v[3].set(ex2, y1, uv.u2, uv.v1, 1.0F, 1.0F, 1.0F, 1.0F);
                v[4].set(x1, y2, uv.u1, uv.v2, 1.0F, 1.0F, 1.0F, 1.0F);
                v[5].set(ex2, y2, uv.u2, uv.v2, 1.0F, 1.0F, 1.0F, 1.0F);
                emojiStagingBuf.append(v, kVerticesPerCell);
            } else {
                const uint32_t renderCp = (cp == 0) ? ' ' : cp;
                const GlyphUV &uv = m_uvCache.value(renderCp, m_spaceUV);
                // isWideAtlasGlyph: NF PUA ranges have 2-cell atlas slots regardless of
                // cell.width (libvterm always reports 1 for PUA via wcwidth).
                // cell.width == 2: fallback for genuinely wide Unicode chars (CJK etc.).
                const auto gx2 = (isWideAtlasGlyph(renderCp) || cell.width == 2)
                    ? static_cast<float>(x1 + 2.0F * static_cast<float>(m_cellWidth))
                    : x2;
                glyphVertSpan[tvi + 0].set(x1,  y1, uv.u1, uv.v1, fgR, fgG, fgB, 1.0F);
                glyphVertSpan[tvi + 1].set(x1,  y2, uv.u1, uv.v2, fgR, fgG, fgB, 1.0F);
                glyphVertSpan[tvi + 2].set(gx2, y1, uv.u2, uv.v1, fgR, fgG, fgB, 1.0F);
                glyphVertSpan[tvi + 3].set(gx2, y1, uv.u2, uv.v1, fgR, fgG, fgB, 1.0F);
                glyphVertSpan[tvi + 4].set(x1,  y2, uv.u1, uv.v2, fgR, fgG, fgB, 1.0F);
                glyphVertSpan[tvi + 5].set(gx2, y2, uv.u2, uv.v2, fgR, fgG, fgB, 1.0F);
                tvi += kVerticesPerCell;
            }
            // NOLINTEND(*-magic-numbers)
        }
    }

    // Trim glyph geometry; upload emoji from staging buffer
    glyphGeom->allocate(tvi);
    const int evi = emojiStagingBuf.size();
    emojiGeom->allocate(evi);
    if (evi > 0)
        memcpy(emojiGeom->vertexData(), emojiStagingBuf.data(), evi * sizeof(GlyphVertex));

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

// NOLINTEND(*-isolate-declaration)