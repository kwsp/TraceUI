#include "EmojiAtlas.h"
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <cmath>

bool isEmoji(uint32_t cp) {
    if (cp == static_cast<uint32_t>(-1)) return false; // libvterm continuation-cell sentinel
    if (cp >= 0xE000  && cp <= 0xF8FF)  return false; // Basic PUA (Nerd Font icons)
    if (cp >= 0xF0000 && cp <= 0xFFFFF) return false; // Supplementary PUA-A (Nerd Fonts v3)
    return (cp >= 0x1F000)
        || (cp >= 0x2600 && cp <= 0x27BF)
        || (cp >= 0x2300 && cp <= 0x23FF);
}

void EmojiAtlas::setCellSize(qreal cellW, qreal cellH, qreal dpr) {
    if (cellW == m_cellW && cellH == m_cellH && dpr == m_dpr)
        return;

    m_cellW = cellW;
    m_cellH = cellH;
    m_dpr = dpr;
    m_emojiSize = cellH;
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

bool EmojiAtlas::ensureGlyph(uint32_t cp) {
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
    font.setFamilies({QStringLiteral("Apple Color Emoji"),
                      QStringLiteral("Noto Color Emoji"),
                      QStringLiteral("Segoe UI Emoji")});
    font.setPixelSize(static_cast<int>(m_emojiSize));

    const QFontMetricsF fm(font);

    QPainter painter(&m_image);
    painter.setFont(font);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
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

GlyphUV EmojiAtlas::uv(uint32_t codepoint) const {
    return m_uvCache.value(codepoint);
}
