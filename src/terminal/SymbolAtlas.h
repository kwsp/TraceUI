#pragma once
#include "GlyphMaterial.h"
#include <QHash>
#include <QImage>

// Returns true for codepoints in the Nerd Fonts v3 Supplementary PUA-A range.
// These cannot be pre-baked (range is ~100k codepoints) so they are rendered lazily.
bool isNerdFontSymbol(uint32_t cp);

class SymbolAtlas {
public:
    // Sets cell dimensions. Resets the atlas if any dimension differs from current.
    // glyphHeight: pixel height for rendered glyphs — pass m_cellHeight so symbols
    // fill the full cell rather than being sized to cap-height like emoji.
    void setCellSize(qreal cellW, qreal cellH, qreal dpr, qreal glyphHeight);

    // Ensures codepoint is in the atlas. Returns true if the atlas image was modified.
    // Returns false on cache hit or if kMaxSlots is exceeded.
    bool ensureGlyph(uint32_t codepoint);

    // UV rect for a codepoint. Only valid after a successful ensureGlyph call.
    [[nodiscard]] GlyphUV uv(uint32_t codepoint) const;

    [[nodiscard]] const QImage &image() const { return m_image; }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markClean() { m_dirty = false; }

private:
    QHash<uint32_t, GlyphUV> m_uvCache;
    QImage m_image; // Format_ARGB32_Premultiplied, white glyphs on transparent
    bool m_dirty{false};
    int m_slotsFilled{0};
    qreal m_cellW{};
    qreal m_cellH{};
    qreal m_dpr{1.0};
    qreal m_glyphSize{};

    static constexpr int kCols = 16;
    static constexpr int kMaxSlots = 512;
    static constexpr int kRows = (kMaxSlots + kCols - 1) / kCols;
};
