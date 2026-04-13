#pragma once
#include "GlyphMaterial.h"
#include <QHash>
#include <QImage>

// Returns true if codepoint should be rendered via the emoji atlas.
bool isEmoji(uint32_t cp);

class EmojiAtlas {
public:
    // Sets cell dimensions. Resets the atlas if any dimension differs from current.
    // emojiHeight: pixel height to render emoji glyphs at — pass QFontMetricsF::capHeight()
    // of the terminal font so emojis match the visual size of capital letters rather than
    // filling the full cell (which includes descent and leading).
    void setCellSize(qreal cellW, qreal cellH, qreal dpr, qreal emojiHeight);

    // Ensures codepoint is in the atlas. Returns true if the atlas image was
    // modified (texture must be re-uploaded). Returns false on cache hit or
    // if kMaxSlots is exceeded.
    bool ensureGlyph(uint32_t codepoint);

    // UV rect for a codepoint. Only valid after a successful ensureGlyph call.
    [[nodiscard]] GlyphUV uv(uint32_t codepoint) const;

    [[nodiscard]] const QImage &image() const { return m_image; }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markClean() { m_dirty = false; }

private:
    QHash<uint32_t, GlyphUV> m_uvCache;
    QImage m_image; // Format_ARGB32_Premultiplied
    bool m_dirty{false};
    int m_slotsFilled{0};
    qreal m_cellW{};
    qreal m_cellH{};
    qreal m_dpr{1.0};
    qreal m_emojiSize{}; // pixel height for emoji glyphs (from terminal font cap height)

    static constexpr int kCols = 16;
    static constexpr int kMaxSlots = 512;
    static constexpr int kRows = (kMaxSlots + kCols - 1) / kCols;
};
