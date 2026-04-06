# TraceUI Implementation Roadmap

## Phase 1: Terminal Support (Complete)
- [x] PTY Backend (macOS/Linux) and `libvterm` integration.
- [x] `TerminalModel` with RichText xterm-256color support.
- [x] `TraceUITerminal` standalone QML module.
- [x] Tiling layout with persistent SplitView state.

## Phase 2: Cyberdeck HUD Refactoring (Complete)
Refactor the entire UI into the high-density HUD layout (System vs Network columns).

### 2.1 Backend Refactoring (C++ & Data Providers)
- [x] **SystemMonitor Update**:
    - [x] Add `osType` (e.g., "MACOS", "LINUX").
    - [x] Add `powerSource` (placeholder "WIRED", add TODO for real battery status).
    - [x] Add `cpuTempCelsius` (ensure real value on macOS/Linux).
    - [x] Add `cpuClockMin` / `cpuClockMax` (placeholders).
    - [x] Add `totalTasks` count (placeholder).
    - [x] Ensure `cpuModelName` is correctly fetched.
- [x] **NetworkMonitor Update**:
    - [x] Add `ipv4Address` string.
    - [x] Add `isOnline` boolean.
    - [x] Add `pingMs` (placeholder).
    - [x] Add `packetLossPct` (placeholder).
    - [x] Coordinate source: Provide Chicago (41.8781, -87.6298) as default lat/lon for the globe.

### 2.2 Styling & Typography
- [x] **Style.qml Expansion**:
    - [x] Define `fontDisplay` (Extra Large for Clock).
    - [x] Define `fontHeader` (Medium/Bold for section titles).
    - [x] Define `fontData` (Small Monospace for metrics).
    - [x] Add `accentSilver` and `textLabel` colors to the palette.
- [x] **Panel Frames**:
    - [x] Create `PanelFrame.qml` component.
    - [x] Implement the "corner bracket" visual style with labels like "SYSTEM", "NETWORK".

### 2.3 System Panel Refactor (Left Column)
- [x] **Header & Clock Section**:
    - [x] Implement XL Digital Clock.
    - [x] Add metadata row: Date, Uptime, OS, Power.
- [x] **CPU Usage Section**:
    - [x] Display CPU model name.
    - [x] Consolidate into a single Avg CPU line graph (reuse `CpuHistoryProvider`).
    - [x] Add sub-stats grid: Temp, Min/Max Clock, Total Tasks.
- [x] **Memory Section**:
    - [x] Text: "USING X OUT OF Y GIB".
    - [x] Visualization: Dot-matrix/Bit-map allocation grid (Custom Shader).
- [x] **Process List Section**:
    - [x] Table headers: PID, NAME, CPU, MEM.
    - [x] High-density list (18px row height).
    - [x] Clickable column headers to toggle sort order.

### 2.4 Network Panel Refactor (Right Column)
- [x] **Network Status Section**:
    - [x] Display IPv4, Online state, Ping.
- [x] **World View Section**:
    - [x] Re-integrate `GlobePanel` as a sub-component.
    - [x] Add "circular target" overlay at the fixed Chicago coordinates.
    - [x] Display Endpoint Lat/Lon text above the globe.
- [x] **Network Traffic Section**:
    - [x] Display Throughput stats: "X MB OUT, Y GB IN".
    - [x] Visual: Large grid-based live waveform (Reuse/Style `NetworkHistoryProvider`).

### 2.5 Global Effects & Polish
- [x] **Scanline Overlay**:
    - [x] Implement a full-screen `ShaderEffect` for the subtle scanline/grid texture.
- [x] **Panel Rearrangement**:
    - [x] Update `Main.qml` default tiling state to present the two-column view.
- [x] **Tiling Persistence**:
    - [x] Ensure the new layout correctly saves/restores via `LayoutStore`.

### 2.6 Startup Animation (Complete)
- [x] **AnimConfig.qml**:
    - [x] Centralized timing configuration singleton.
    - [x] Configurable durations for all animation phases.
- [x] **Terminal Reveal**:
    - [x] Horizontal line appears at terminal position.
    - [x] Line expands upward to reveal terminal content.
- [x] **Panel Fade-in**:
    - [x] System and Network panels fade in after terminal reveal.
- [x] **Globe Intro**:
    - [x] Globe intro animation starts after panels are visible.
    - [x] Uses Globe's built-in `startupDelay` property.

## Phase 3: Advanced Terminal Features (Current)
- [ ] **Scrollback Buffer**:
    - [ ] Implement ring buffer in `TerminalBackend` using `sb_pushline` / `sb_popline` callbacks.
- [ ] **Alt-Screen Support**:
    - [ ] Handle alternate screen buffer transitions for TUI apps (vim/top).
- [ ] **Native GPU Rendering (RHI)**:
    - [ ] **Terminal Renderer Component**: Create `TerminalRenderer` as a `QQuickItem` subclass.
    - [ ] **Glyph Atlas**: Implement a texture atlas for monospace glyphs (pre-rendered via `QRawFont` or `FreeType`).
    - [ ] **Grid Geometry**: Use `QSGGeometryNode` to manage a grid of textured quads.
    - [ ] **Cell Data Buffer**: Map `libvterm` screen state directly to a GPU vertex buffer (Cell attributes: UVs, FG/BG colors).
    - [ ] **RHI Shaders**: Implement vertex/fragment shaders for high-performance cell compositing (bypass QML RichText).
- [ ] Windows ConPTY support.

## Phase 4: Real Backend Data
- [x] **SystemMonitor** — replace all placeholders with real data:
    - [x] `powerSource`: IOPowerSources API (AC / BAT nn%).
    - [x] `cpuClockMin/Max`: sysctl hw.cpufrequency, Apple Silicon fallback.
    - [x] `totalTasks`: sysctl KERN_PROC_ALL count.
    - [x] `cpuTempCelsius`: AppleSMC via IOKit (TC0P/TC0D/Tp09/Tp01).
- [x] **NetworkMonitor** — replace all placeholders with real data:
    - [x] `ipv4Address`: getifaddrs() scanning AF_INET on real interfaces.
    - [x] `isOnline`: derived from ipv4Address availability.
    - [x] `pingMs`: async `ping -c1` via QProcess, parsed RTT.
    - [x] `packetLossPct`: sliding-window loss tracking.
    - [x] `activeConnections`: net.inet.tcp.pcbcount sysctl.
- [ ] GeoIP integration for globe endpoint coordinates.
