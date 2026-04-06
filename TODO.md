# TraceUI Implementation Roadmap

## Phase 1: Terminal Support (Complete)
- [x] PTY Backend (macOS/Linux) and `libvterm` integration.
- [x] `TerminalModel` with RichText xterm-256color support.
- [x] `TraceUITerminal` standalone QML module.
- [x] Tiling layout with persistent SplitView state.

## Phase 2: Cyberdeck HUD Refactoring (Current)
Refactor the entire UI into the high-density HUD layout (System vs Network columns).

### 2.1 Backend Refactoring (C++ & Data Providers)
- [ ] **SystemMonitor Update**:
    - [ ] Add `osType` (e.g., "MACOS", "LINUX").
    - [ ] Add `powerSource` (placeholder "WIRED", add TODO for real battery status).
    - [ ] Add `cpuTempCelsius` (ensure real value on macOS/Linux).
    - [ ] Add `cpuClockMin` / `cpuClockMax` (placeholders).
    - [ ] Add `totalTasks` count (placeholder).
    - [ ] Ensure `cpuModelName` is correctly fetched.
- [ ] **NetworkMonitor Update**:
    - [ ] Add `ipv4Address` string.
    - [ ] Add `isOnline` boolean.
    - [ ] Add `pingMs` (placeholder).
    - [ ] Add `packetLossPct` (placeholder).
    - [ ] Coordinate source: Provide Chicago (41.8781, -87.6298) as default lat/lon for the globe.

### 2.2 Styling & Typography
- [ ] **Style.qml Expansion**:
    - [ ] Define `fontDisplay` (Extra Large for Clock).
    - [ ] Define `fontHeader` (Medium/Bold for section titles).
    - [ ] Define `fontData` (Small Monospace for metrics).
    - [ ] Add `accentSilver` and `textLabel` colors to the palette.
- [ ] **Panel Frames**:
    - [ ] Create `PanelFrame.qml` or `PanelBracket.qml` component.
    - [ ] Implement the "corner bracket" visual style with labels like "SYSTEM", "NETWORK".

### 2.3 System Panel Refactor (Left Column)
- [ ] **Header & Clock Section**:
    - [ ] Implement XL Digital Clock.
    - [ ] Add metadata row: Date, Uptime, OS, Power.
- [ ] **CPU Usage Section**:
    - [ ] Display CPU model name.
    - [ ] Consolidate into a single Avg CPU line graph (reuse `CpuHistoryProvider`).
    - [ ] Add sub-stats grid: Temp, Min/Max Clock, Total Tasks.
- [ ] **Memory Section**:
    - [ ] Text: "USING X OUT OF Y GIB".
    - [ ] Visualization: Dot-matrix/Bit-map allocation grid (Custom Shader or Grid of Rects).
- [ ] **Process List Section**:
    - [ ] Table headers: PID, NAME, CPU, MEM.
    - [ ] High-density list (20px row height).

### 2.4 Network Panel Refactor (Right Column)
- [ ] **Network Status Section**:
    - [ ] Display IPv4, Online state, IP, and Ping.
- [ ] **World View Section**:
    - [ ] Re-integrate `GlobePanel` as a sub-component.
    - [ ] Add "circular target" overlay at the fixed Chicago coordinates.
    - [ ] Display Endpoint Lat/Lon text above the globe.
- [ ] **Network Traffic Section**:
    - [ ] Display Throughput stats: "X MB OUT, Y GB IN".
    - [ ] Visual: Large grid-based live waveform (Reuse/Style `NetworkHistoryProvider`).
    - [ ] Add Y-axis labels (0.61 to -1.61) for aesthetic accuracy.

### 2.5 Global Effects & Polish
- [ ] **Scanline Overlay**:
    - [ ] Implement a full-screen `ShaderEffect` for the subtle scanline/grid texture.
- [ ] **Panel Rearrangement**:
    - [ ] Update `Main.qml` default tiling state to present the two-column view.
- [ ] **Tiling Persistence**:
    - [ ] Ensure the new layout correctly saves/restores via `LayoutStore`.

## Phase 3: Advanced Terminal Features (Future)
- [ ] Scrollback buffer ring.
- [ ] Alt-screen support (vim/top).
- [ ] Windows ConPTY support.
