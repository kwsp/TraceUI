#include "SymbolAtlas.h"
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <cmath>

bool isNerdFontSymbol(uint32_t cp) {
    // Nerd Fonts v3 Supplementary PUA-A range (U+F0001–U+F1AF0).
    // These codepoints are absent from the pre-baked kAtlasRanges in TerminalRenderer
    // and are rendered here lazily on first encounter.
    return cp >= 0xF0001 && cp <= 0xF1AF0;
}

void SymbolAtlas::setCellSize(qreal cellW, qreal cellH, qreal dpr, qreal glyphHeight) {
    if (cellW == m_cellW && cellH == m_cellH && dpr == m_dpr && glyphHeight == m_glyphSize)
        return;

    m_cellW = cellW;
    m_cellH = cellH;
    m_dpr = dpr;
    m_glyphSize = glyphHeight;
    m_uvCache.clear();
    m_slotsFilled = 0;

    constexpr qreal kMargin = 1.0;
    const qreal slotW = 2.0 * cellW + 2.0 * kMargin;
    const qreal slotH = cellH + 2.0 * kMargin;

    const int physW = static_cast<int>(std::ceil(kCols * slotW * dpr));
    const int physH = static_cast<int>(std::ceil(kRows * slotH * dpr));

    m_image = QImage(physW, physH, QImage::Format_ARGB32_Premultiplied);
    m_image.setDevicePixelRatio(dpr);
    m_image.fill(Qt::transparent);
    m_dirty = true;
}

bool SymbolAtlas::ensureGlyph(uint32_t cp) {
    if (m_uvCache.contains(cp))
        return false;
    if (m_slotsFilled >= kMaxSlots)
        return false;
    if (m_image.isNull())
        return false;

    constexpr qreal kMargin = 1.0;
    const qreal slotW = 2.0 * m_cellW + 2.0 * kMargin;
    const qreal slotH = m_cellH + 2.0 * kMargin;
    const qreal atlasLogW = kCols * slotW;
    const qreal atlasLogH = kRows * slotH;

    const int slot = m_slotsFilled++;
    const int col = slot % kCols;
    const int row = slot / kCols;

    const qreal logX = col * slotW;
    const qreal logY = row * slotH;

    QFont font;
    font.setFamilies({QStringLiteral("Symbols Nerd Font Mono")});
    font.setPixelSize(static_cast<int>(m_glyphSize));

    const QFontMetricsF fm(font);

    QPainter painter(&m_image);
    painter.setFont(font);
    // White pen: glyph alpha is in texel.a, which the glyph shader multiplies by
    // the per-vertex foreground colour. This gives correct fg-colour tinting.
    painter.setPen(Qt::white);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const char32_t ucs4cp = static_cast<char32_t>(cp);
    const QString str = QString::fromUcs4(&ucs4cp, 1);
    painter.drawText(QPointF(logX + kMargin, logY + kMargin + fm.ascent()), str);
    painter.end();

    const GlyphUV uv{
        .u1 = static_cast<float>((logX + kMargin) / atlasLogW),
        .v1 = static_cast<float>((logY + kMargin) / atlasLogH),
        .u2 = static_cast<float>((logX + kMargin + 2.0 * m_cellW) / atlasLogW),
        .v2 = static_cast<float>((logY + kMargin + m_cellH) / atlasLogH),
    };

    m_uvCache[cp] = uv;
    m_dirty = true;
    return true;
}

GlyphUV SymbolAtlas::uv(uint32_t codepoint) const {
    return m_uvCache.value(codepoint);
}
