#include "GlyphAtlas.h"
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <cmath>

bool isWideIcon(uint32_t cp) {
    // Nerd Font v3 BMP ranges
    if (cp >= 0xE000 && cp <= 0xE00A) return true;
    if (cp >= 0xE0A0 && cp <= 0xE0D7) return true;
    if (cp >= 0xE200 && cp <= 0xE2A9) return true;
    if (cp >= 0xE300 && cp <= 0xE3E3) return true;
    if (cp >= 0xE5FA && cp <= 0xE6B7) return true;
    if (cp >= 0xE700 && cp <= 0xE8EF) return true;
    if (cp >= 0xEA60 && cp <= 0xEC1E) return true;
    if (cp >= 0xED00 && cp <= 0xEFCE) return true;
    if (cp >= 0xF000 && cp <= 0xF533) return true;
    // Nerd Fonts v3 Supplementary PUA-A range
    if (cp >= 0xF0001 && cp <= 0xF1AF0) return true;
    return false;
}

void GlyphAtlas::setCellSize(qreal cellW, qreal cellH, qreal dpr, qreal ascent, const QString &fontFamily, int fontSize) {
    if (cellW == m_cellW && cellH == m_cellH && dpr == m_dpr && ascent == m_ascent && fontFamily == m_fontFamily && fontSize == m_fontSize)
        return;

    m_cellW = cellW;
    m_cellH = cellH;
    m_dpr = dpr;
    m_ascent = ascent;
    m_fontFamily = fontFamily;
    m_fontSize = fontSize;
    m_uvCache.clear();
    m_currentSlot = 0;

    constexpr qreal kMargin = 1.0;
    const qreal slotW = m_cellW + 2.0 * kMargin;
    const qreal slotH = m_cellH + 4.0 * kMargin;

    const int physW = static_cast<int>(std::ceil(kCols * slotW * dpr));
    const int physH = static_cast<int>(std::ceil(kRows * slotH * dpr));

    m_image = QImage(physW, physH, QImage::Format_ARGB32_Premultiplied);
    m_image.setDevicePixelRatio(dpr);
    m_image.fill(Qt::transparent);

    // Pre-render ASCII + Latin-1 (0x20 - 0xFF)
    for (uint32_t cp = 0x20; cp <= 0xFF; ++cp) {
        ensureGlyph(cp, 1);
    }
    
    m_dirty = true;
}

bool GlyphAtlas::ensureGlyph(uint32_t cp, int cellWidth) {
    if (m_uvCache.contains(cp))
        return false;

    uint32_t renderCp = (cp == 0) ? ' ' : cp;
    if (m_uvCache.contains(renderCp)) {
        m_uvCache[cp] = m_uvCache[renderCp];
        return false;
    }

    const int slotsNeeded = (cellWidth == 2 || isWideIcon(renderCp)) ? 2 : 1;
    
    // Wrap wide glyphs to next row if they'd overflow
    if (slotsNeeded == 2 && (m_currentSlot % kCols) == kCols - 1) {
        m_currentSlot++;
    }

    if (m_currentSlot + slotsNeeded > kMaxSlots) return false;

    renderGlyph(renderCp, slotsNeeded);
    
    // Cache both the requested CP and the rendered CP
    m_uvCache[cp] = m_uvCache[renderCp];
    m_currentSlot += slotsNeeded;
    m_dirty = true;
    return true;
}

void GlyphAtlas::renderGlyph(uint32_t cp, int slotsNeeded) {
    constexpr qreal kMargin = 1.0;
    const qreal slotW = m_cellW + 2.0 * kMargin;
    const qreal slotH = m_cellH + 4.0 * kMargin;
    const qreal atlasLogW = kCols * slotW;
    const qreal atlasLogH = kRows * slotH;

    const int col = m_currentSlot % kCols;
    const int row = m_currentSlot / kCols;

    const qreal logX = col * slotW;
    const qreal logY = row * slotH;
    const qreal drawW = slotsNeeded * m_cellW;

    QFont font(m_fontFamily, m_fontSize);
    font.setFamilies({m_fontFamily, QStringLiteral("Symbols Nerd Font Mono")});

    QPainter painter(&m_image);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const auto ucs4cp = static_cast<char32_t>(cp);
    const QString str = QString::fromUcs4(&ucs4cp, 1);
    // Center text vertically in the slot (which has 2.0 margin top/bottom)
    painter.drawText(QPointF(logX + kMargin, logY + 2.0 * kMargin + m_ascent), str);
    painter.end();

    m_uvCache[cp] = GlyphUV{
        .u1 = static_cast<float>((logX + kMargin) / atlasLogW),
        .v1 = static_cast<float>((logY + 2.0 * kMargin) / atlasLogH),
        .u2 = static_cast<float>((logX + kMargin + drawW) / atlasLogW),
        .v2 = static_cast<float>((logY + 2.0 * kMargin + m_cellH) / atlasLogH),
    };
}

GlyphUV GlyphAtlas::uv(uint32_t codepoint) const {
    return m_uvCache.value(codepoint);
}
