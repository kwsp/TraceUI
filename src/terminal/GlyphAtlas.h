#pragma once
#include "GlyphMaterial.h"
#include <QHash>
#include <QImage>
#include <QString>

// Returns true for codepoints that should be rendered 2-cells wide (Nerd Fonts).
bool isWideIcon(uint32_t cp);

class GlyphAtlas {
public:
    // Configures metrics and font. Resets the atlas if any parameter differs.
    void setCellSize(qreal cellW, qreal cellH, qreal dpr, qreal ascent, const QString &fontFamily,
                     int fontSize);

    // Ensures codepoint is in the atlas. cellWidth is the vterm reported width (1 or 2).
    // Returns true if a new glyph was rendered into the atlas.
    bool ensureGlyph(uint32_t codepoint, int cellWidth);

    // UV rect for a codepoint. Only valid after ensureGlyph.
    [[nodiscard]] GlyphUV uv(uint32_t codepoint) const;

    [[nodiscard]] const QImage &image() const { return m_image; }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markClean() { m_dirty = false; }

private:
    void renderGlyph(uint32_t cp, int slotsNeeded);

    QHash<uint32_t, GlyphUV> m_uvCache;
    QImage m_image;
    bool m_dirty{false};

    int m_currentSlot{0};

    qreal m_cellW{};
    qreal m_cellH{};
    qreal m_dpr{1.0};
    qreal m_ascent{};
    QString m_fontFamily;
    int m_fontSize{0};

    static constexpr int kCols = 64;
    static constexpr int kMaxSlots = 4096;
    static constexpr int kRows = (kMaxSlots + kCols - 1) / kCols;
};
