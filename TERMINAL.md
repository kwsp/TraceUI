# Terminal Emulation and Performance Notes

This document contains notes on terminal emulator performance, focusing on architectural patterns observed in high-performance terminals like `kitty`.

## Architecture of High-Performance Terminals (e.g., Kitty)

High-performance terminal emulators handle two distinct but related tasks: parsing vast amounts of incoming data (throughput) and presenting updates to the screen smoothly without noticeable lag (latency). Typical UI frameworks (like Qt/QML's `Text` element or DOM elements in a browser) are generally not designed for the specific demands of terminal rendering, often resulting in performance bottlenecks.

### 1. Separation of Concerns (Threading)
Kitty rigorously separates I/O and parsing from rendering.
- **I/O Thread**: Reads from the PTY, processes signals, and parses the byte stream into terminal state changes.
- **Render Loop / Main Thread**: Pulls the current state of the grid at a fixed interval (often synced to the monitor refresh rate, e.g., 60-120fps) and renders it.
- **Result**: The parser never blocks on the GPU/UI, and the GPU never blocks on slow shell processes.

### 2. Throttling and Batching
To balance CPU usage with perceived latency, Kitty employs artificial delays.
- **`input_delay` (e.g., 3ms)**: Wait a few milliseconds after reading data to see if more data arrives before waking up the main loop. This batches thousands of fast PTY reads into a single UI update frame, drastically reducing CPU load during `cat largefile.txt`.
- **`repaint_delay` (e.g., 10ms)**: Enforces a minimum time between screen paints to cap the maximum FPS and avoid wasting CPU/GPU cycles on frames the monitor cannot display.
- **Synchronized Updates**: Supports DCS sequences (e.g., `BSU`/`ESU`) allowing CLI apps to explicitly pause rendering until a full frame is drawn (useful for `tmux` or `neovim`).

### 3. GPU Rendering and Glyph Caching
Kitty bypasses standard OS text rendering libraries for the hot path.
- **Glyph Cache / Texture Atlas**: Characters are rasterized via FreeType/CoreText *once* per font/size combination and uploaded to a GPU texture atlas.
- **Cell-Based Rendering**: The screen is a grid of cells. Each cell contains a foreground color, background color, and a UV coordinate pointing to its pre-rendered glyph in the texture atlas.
- **Shader Pipeline**: A vertex shader generates the grid, and a fragment shader composites the background, glyph texture, and text colors directly on the GPU.

### 4. SIMD Parsing
Parsing the ANSI escape sequences and UTF-8 byte stream is extremely performance-sensitive. Kitty uses Single Instruction, Multiple Data (SIMD) vector instructions to scan the input stream for escape characters (`\x1b`), CR/LF, and UTF-8 boundaries, parsing dozens of bytes per CPU cycle.

### 5. Efficient State Representation
- **Fixed-size Arrays**: The screen buffer is represented by flat, pre-allocated C arrays (`LineBuf`) rather than trees or objects. Memory is rarely allocated during normal operation.
- **Dirty Tracking**: Lines and individual cells are marked "dirty" when modified. The renderer only uploads dirty regions to the GPU (via `glTexSubImage2D` or mapped buffers), avoiding full-screen updates.

## Implications for TraceUI Terminal

Currently, the `TraceUI` terminal relies on a `ListView` mapping to QML `Text` delegates containing `Text.RichText`. While idiomatic for typical desktop UI, this approach has scaling limits:

1. **Rich Text Overhead**: Parsing HTML strings (e.g., `<span style="...">`) inside Qt for 50-100 lines on every keystroke or screen update is very CPU intensive.
2. **Layout Thrashing**: `ListView` and `Text` elements invoke Qt's heavy layout engines, text shaping, and rendering paths.
3. **Over-Rendering**: Emitting `dataChanged` across the whole view currently forces QML to rebuild delegates for unchanged lines.

### Potential Future Improvements for TraceUI

To approach native terminal performance, TraceUI could progressively implement:
1. **Targeted Dirty Tracking**: Modify `TerminalBackend` and `TerminalModel` to only emit `dataChanged` for lines that were *actually* modified by `vterm_screen_flush_damage`, instead of `(0, rowCount() - 1)`.
2. **Debounce Updates**: Accumulate PTY events in `TerminalBackend` and use a `QTimer` (e.g., 16ms/60fps) to flush updates to the QML model in batches, similar to Kitty's `input_delay`.
3. **Custom QQuickItem Renderer**: Bypass QML `ListView` entirely. Implement a custom `QQuickItem` subclass in C++ that uses `QSGNode` to render the grid of cells.
4. **Direct OpenGL/Vulkan**: For ultimate performance, implement a QML wrapper around a raw OpenGL context (or port a library like `alacritty` or `wezterm` core) managing its own glyph atlas, completely sidestepping Qt's SceneGraph for the terminal panel.
