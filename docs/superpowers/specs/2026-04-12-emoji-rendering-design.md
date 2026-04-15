# Emoji Rendering in TerminalRenderer

**Date:** 2026-04-12
**Scope:** Best-effort emoji support for the GPU terminal renderer

---

## Goal

Render common emoji codepoints in the terminal using a separate RGBA atlas and passthrough shader, without touching the existing text rendering path. Emoji occupy 2-wide cells matching libvterm's wide-character layout.

---

## Architecture

The scene graph node tree gains a third child:

```
rootNode
  ├── bgNode      (QSGVertexColorMaterial — cell background colors, unchanged)
  ├── glyphNode   (GlyphMaterial — alpha atlas, tinted text glyphs, unchanged)
  └── emojiNode   (EmojiMaterial — RGBA atlas, passthrough full-color emoji)
```

The cell iteration loop in `updatePaintNode` routes each cell to either the glyph path or the emoji path based on `isEmoji(codepoint)`. Background quads are emitted for all cells regardless.

---

## Emoji Detection

```cpp
static bool isEmoji(uint32_t cp) {
    return (cp >= 0x1F000)                  // main emoji blocks
        || (cp >= 0x2600 && cp <= 0x27BF)   // misc symbols & dingbats
        || (cp >= 0x2300 && cp <= 0x23FF);  // misc technical (⏰ ⌛ etc.)
}
```

Cells where `chars[0] == 0` (wide-cell continuation) are skipped in both passes. Wide emoji at column C produce a quad spanning `[x, x + 2*cellWidth]`; the existing `c += cell.width - 1` skip handles the continuation cell.

Cells with an emoji codepoint that exceeds `EmojiAtlas::kMaxSlots` silently degrade to background-color-only (no glyph rendered).

---

## kAtlasRanges update

Remove `{0x2600, 0x26FF}` from `kAtlasRanges` — those codepoints now route to the emoji atlas. `totalGlyphCount()` and `rebuildAtlas()` both iterate over the array and auto-update.

```cpp
static constexpr CodepointRange kAtlasRanges[] = {
    {0x0020, 0x007E}, // ASCII printable (95)
    {0x00A0, 0x00FF}, // Latin-1 Supplement (96)
    {0x2500, 0x257F}, // Box Drawing (128)
    {0x2580, 0x259F}, // Block Elements (32)
    {0x2190, 0x21FF}, // Arrows (112)
    {0x25A0, 0x25FF}, // Geometric Shapes (96)
    // {0x2600, 0x26FF} removed — now handled by emoji atlas
};
```

---

## EmojiAtlas

New class (`EmojiAtlas.h` / `EmojiAtlas.cpp`) with single responsibility: manages the RGBA emoji glyph atlas.

```cpp
class EmojiAtlas {
public:
    void setCellSize(qreal w, qreal h, qreal dpr);

    // Returns true if atlas image was modified (texture needs re-upload).
    bool ensureGlyph(uint32_t codepoint);

    GlyphUV uv(uint32_t codepoint) const;
    const QImage &image() const;
    bool isDirty() const;
    void markClean();

private:
    QHash<uint32_t, GlyphUV> m_uvCache;
    QImage m_atlasImage;   // Format_ARGB32_Premultiplied
    bool m_dirty{false};
    int m_slotsFilled{0};
    qreal m_cellW{}, m_cellH{}, m_dpr{1.0};

    static constexpr int kCols = 16;
    static constexpr int kMaxSlots = 512;
};
```

**Atlas layout:** Each slot is `2*cellWidth × cellHeight` (emoji are always wide). 16 columns, rows grow as slots fill. At 14pt on a Retina display this is approximately 640×640 physical pixels.

**Font selection:** Qt picks the platform emoji font via fallback families:
```cpp
QFont font;
font.setFamilies({"Apple Color Emoji", "Noto Color Emoji", "Segoe UI Emoji"});
```

**Lazy population:** `ensureGlyph()` is a no-op on cache hit. On miss, it renders via `QPainter::drawText()` into the next free slot, stores the UV, and sets `m_dirty = true`. Past `kMaxSlots`, it returns `false` and the caller skips the emoji draw.

**Invalidation:** `setCellSize()` clears `m_uvCache`, resets `m_slotsFilled`, and sets `m_dirty = true`. Called from `TerminalRenderer` alongside the text atlas invalidation in `setFontFamily` / `setFontSize`.

---

## EmojiMaterial + Shader

`EmojiMaterial` mirrors `GlyphMaterial` in structure but has its own `QSGMaterialType` key so Qt's scene graph treats them as distinct materials.

New files: `EmojiMaterial.h`, `EmojiMaterial.cpp`, `shaders/emoji.vert`, `shaders/emoji.frag`.

The vertex shader is identical to `glyph.vert`. The fragment shader drops color tinting:

```glsl
// emoji.frag
void main() {
    fragColor = texture(emojiAtlas, vTexCoord) * qt_Opacity;
}
```

The existing `GlyphVertex` layout (x, y, u, v, r, g, b, a) is reused for the emoji geometry node. The color attributes are present but unused in the emoji shader — no vertex struct changes.

---

## TerminalRenderer Changes

**New members:**
```cpp
EmojiAtlas m_emojiAtlas;
QSGTexture *m_emojiTexture{};  // owned by scene graph
```

**`setFontFamily` / `setFontSize`:** call `m_emojiAtlas.setCellSize(m_cellWidth, m_cellHeight, dpr)` after `recalcMetrics()`.

**`updatePaintNode`:** node tree becomes three children (bgNode, glyphNode, emojiNode). The cell loop uses two independent vertex indices (`tvi` for text, `evi` for emoji). Both geometry buffers are pre-allocated to `cellCount * 6` (upper bound) then trimmed with `allocate(tvi)` / `allocate(evi)` after the loop.

Cell routing:

```cpp
const uint32_t cp = cell.chars[0];
if (isEmoji(cp) && cp != 0) {
    if (m_emojiAtlas.ensureGlyph(cp)) { /* atlas grew, texture needs re-upload */ }
    const GlyphUV uv = m_emojiAtlas.uv(cp);
    const float ex2 = x1 + 2.0F * static_cast<float>(m_cellWidth);
    // fill emojiVertSpan[evi .. evi+5] with 2-wide quad
    evi += 6;
} else {
    const GlyphUV &uv = m_uvCache.value(cp == 0 ? ' ' : cp, m_spaceUV);
    // fill glyphVertSpan[tvi .. tvi+5] as before
    tvi += 6;
}
if (cell.width > 1)
    c += cell.width - 1;
```

After the loop, if `m_emojiAtlas.isDirty()`, the emoji texture is re-created from the updated atlas image before `emojiNode->markDirty(...)`.

---

## Error Handling

- Emoji past `kMaxSlots`: silently skip glyph draw; background color still renders.
- Emoji font unavailable (no system emoji font): `QPainter::drawText` produces a blank slot; degrades gracefully to background-only.
- No crashes, no assertions — all failure modes produce invisible output.

---

## Files Added / Modified

| File | Change |
|---|---|
| `src/terminal/EmojiAtlas.h` | New |
| `src/terminal/EmojiAtlas.cpp` | New |
| `src/terminal/EmojiMaterial.h` | New |
| `src/terminal/EmojiMaterial.cpp` | New |
| `src/terminal/shaders/emoji.vert` | New (copy of glyph.vert) |
| `src/terminal/shaders/emoji.frag` | New (passthrough) |
| `src/terminal/TerminalRenderer.h` | Add `m_emojiAtlas`, `m_emojiTexture` |
| `src/terminal/TerminalRenderer.cpp` | Update `kAtlasRanges`, add routing logic, third node |
| `src/terminal/CMakeLists.txt` | Add new source files and shaders |
