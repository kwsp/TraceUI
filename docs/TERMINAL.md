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

## Current Implementation Status in TraceUI

The TraceUI terminal is built using `libvterm` for terminal state emulation and a QML `ListView` for rendering.

### Key Architectural Decisions
- **`TerminalBackend`**: Manages the PTY lifecycle (`forkpty`) and bridges `libvterm` with Qt.
- **`TerminalModel`**: A `QAbstractListModel` that transforms the `libvterm` screen buffer into HTML-formatted lines.
- **Rich Text Rendering**: Each terminal row is rendered as a QML `Text` element with `textFormat: Text.RichText`. Per-cell foreground/background colors and attributes (bold, italic, underline) are supported by grouping character runs into `<span>` tags.
- **Performance Optimizations**:
    - **Targeted Dirty Tracking**: `TerminalBackend` tracks the exact range of modified rows via `libvterm`'s damage callbacks. `TerminalModel` only emits `dataChanged` for modified rows, preventing unnecessary delegate rebuilds.
    - **Debounced Updates**: Damage events are coalesced and flushed to the UI at a maximum rate of 60 FPS using a `QTimer`. This prevents the UI from being overwhelmed by high-throughput PTY data.
- **System Integration**:
    - Automatic detection of the user's login shell via `getpwuid`.
    - Shell starts as a login shell (prefixed with `-`).
    - Working directory is set to the user's home directory on startup.

## Limitations and Future Work

While the current implementation is functional and optimized for a `ListView` approach, it still relies on Qt's rich text layout engine, which has inherent performance limits for high-frequency updates (e.g., full-screen TUI apps like `vim` or `top`).

### Potential Future Improvements

1. **Custom QQuickItem Renderer**: Replace `ListView` and `RichText` with a C++ `QQuickItem` subclass. Use `QSGNode` (Qt Scene Graph) to render the grid of cells directly, using a texture atlas for glyphs. This would drastically reduce CPU overhead and layout thrashing.
2. **Scrollback Buffer**: Currently, lines that scroll off the top are lost. Implementing a scrollback ring-buffer in `TerminalBackend` would allow for historical viewing.
3. **Mouse Support**: Forwarding mouse events from QML to the PTY would enable mouse interaction in terminal applications that support it.
4. **Alt-Screen Support**: Explicitly handling the alternate screen buffer for full-screen applications.
5. **Direct OpenGL/Vulkan**: For ultimate performance, bypass the Qt Scene Graph entirely for the terminal grid.
