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

## Phase 3: Advanced Terminal Features (Future)
- [ ] Scrollback buffer ring.
- [ ] Alt-screen support (vim/top).
- [ ] Windows ConPTY support.

## Phase 4: Real Backend Data (Future)
- [ ] Implement actual data fetching for `powerSource`, `cpuClockMin/Max`.
- [ ] Implement actual `pingMs`, `packetLossPct` measurements.
- [ ] GeoIP integration for globe endpoint coordinates.
