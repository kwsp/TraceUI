# Terminal Emulation and Performance

This document describes the architecture of the TraceUI terminal emulator, focusing on how it achieves high performance through efficient state management and GPU-accelerated rendering.

## Architecture Overview

TraceUI's terminal is built to handle high-throughput data and low-latency rendering. It separates terminal state management from the rendering pipeline.

### 1. Terminal State Management (`TerminalBackend`)
The `TerminalBackend` is the core of the terminal's logic. It manages:
- **PTY Lifecycle**: Handles `forkpty`, opening the master/slave PTY pairs, and managing the child process (typically a shell).
- **I/O Handling**: Uses `QSocketNotifier` to asynchronously read from the PTY as data becomes available.
- **State Emulation (`libvterm`)**: Uses the `libvterm` library to parse the byte stream (ANSI/VT escape sequences) and maintain the internal screen buffer (grid of cells, colors, attributes).

### 2. GPU-Accelerated Rendering (`TerminalRenderer`)
Instead of using standard UI elements (like `ListView` or `Text`), TraceUI uses a custom `QQuickItem` for rendering the terminal grid.

- **Glyph Caching / Texture Atlas**: Characters are rasterized via `QRawFont` once and uploaded to a GPU texture atlas. This avoids expensive text layout and rasterization on every frame.
- **Cell-Based Grid**: The screen is represented as a grid of cells. Each cell contains foreground/background colors and a UV coordinate pointing to its pre-rendered glyph in the atlas.
- **Shader Pipeline**: A custom vertex shader generates the grid geometry, and a fragment shader composites the background and text colors with the glyph texture directly on the GPU.
- **Efficient Buffer Updates**: Only the "dirty" regions of the screen (tracked via `libvterm` damage callbacks) are updated in the renderer's geometry nodes.

### 3. Performance Optimizations

#### Throttling and Batching
To balance CPU usage with perceived latency, `TerminalBackend` employs debouncing:
- **Coalesced Damage**: `libvterm` damage events are collected and only flushed to the UI at a maximum rate of 60 FPS using a `QTimer`.
- **Batch Processing**: Multiple PTY reads are processed into the `libvterm` state before triggering a UI update, significantly reducing overhead during high-volume output (e.g., `cat largefile.txt`).

#### Efficient Data Access
- **Direct Screen Access**: `TerminalRenderer` accesses the `libvterm` screen buffer directly to populate GPU buffers, avoiding intermediate string allocations or HTML formatting.
- **Minimal Memory Allocation**: The terminal grid uses pre-allocated buffers where possible.

## Current Implementation Status

- **Shell Integration**: Automatically detects the user's login shell and starts it as a login shell (prefixed with `-`).
- **Resizing**: Properly forwards window size changes to the PTY and `libvterm`.
- **Colors & Attributes**: Full support for standard ANSI colors and text attributes (bold, etc.) via the shader pipeline.

## Future Work

1. **Scrollback Buffer**: Implementing a scrollback ring-buffer in `TerminalBackend` to allow historical viewing.
2. **Mouse Support**: Forwarding mouse events from QML to the PTY for interactive terminal apps.
3. **Alt-Screen Support**: Explicitly handling the alternate screen buffer for full-screen applications like `vim` or `htop`.
4. **SIMD Parsing**: Exploring SIMD optimizations for the input stream if `libvterm` parsing becomes a bottleneck.
