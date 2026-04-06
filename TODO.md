# TraceUI Terminal Implementation TODO

## 1. UI Restructuring
- [x] Shrink `GlobePanel` and position it next to `SystemPanel`.
- [x] Move `ProcessPanel` and `NetworkPanel` to maintain the left-hand status column.
- [x] Create a placeholder `TerminalPanel` occupying the right portion of the screen.

## 2. Terminal Backend (C++ & libvterm)
- [x] Add `libvterm` as a submodule or external dependency.
- [x] Implement `TerminalBackend` class:
    - [x] Manage PTY lifecycle (macOS/Linux `forkpty`).
    - [x] Setup `QSocketNotifier` to monitor PTY for incoming data.
    - [x] Initialize `VTerm` and `VTermScreen`.
    - [x] Implement `libvterm` callbacks (damage, movecursor, settermprop).
    - [x] Handle input: Forward keyboard events from QML to PTY.
    - [x] RAII wrappers for VTerm (`VTermPtr`) and file descriptors (`FileDescriptor`).
    - [x] Proper child process cleanup (SIGHUP + waitpid) to prevent zombies.
    - [x] Coalesced `resize(rows, cols)` to avoid double TIOCSWINSZ.
    - [x] Auto-detect user's login shell via `getpwuid`.
    - [x] Handle double-width characters in `getLineText`.
    - [x] `vterm_screen_flush_damage` after PTY reads for reliable rendering.
    - [x] `vterm_output_set_callback` for terminal-to-shell control sequences.
- [x] Implement `TerminalModel` (QAbstractListModel):
    - [x] Map `libvterm` screen buffer to model rows.
    - [x] Expose cell data (character role; foreground/background roles stubbed).
    - [x] Signal `dataChanged` with specific roles on screen updates.
- [x] Unit Tests for `TerminalBackend` and `TerminalModel` (11 tests).

## 3. Terminal Frontend (QML)
- [x] Create `TerminalPanel.qml` with `FocusScope`.
- [x] Grid-based text rendering using monospaced font (`Hack`).
- [x] Keyboard interaction (`Keys.onPressed`) with Ctrl, arrows, Home/End/Delete/PgUp/PgDn.
- [x] Visual cursor (blinking, positioned relative to scroll offset).
- [x] Terminal resizing when window dimensions change (coalesced via `resize()`).
- [x] Click-to-focus with `MouseArea`.

## 4. Remaining Work

### Per-Cell Color Support
- [ ] Implement `ForegroundRole` / `BackgroundRole` in `TerminalModel::data()`.
- [ ] Extract `VTermScreenCell` fg/bg colors and map to Qt colors.
- [ ] Update delegate to use per-cell or per-run coloring (requires `Repeater` or custom render).

### Scrollback Buffer
- [ ] Implement `sb_pushline` / `sb_popline` callbacks in `TerminalBackend`.
- [ ] Store scrollback lines in a ring buffer.
- [ ] Allow scrolling back through history (mouse wheel or Shift+PgUp/PgDn).

### Alt-Screen Support
- [ ] Call `vterm_screen_enable_altscreen()` for programs like `vim`, `top`, `less`.

### Styling & Integration
- [ ] Apply Neo-Kitsch theme colors to the terminal's default palette via `vterm_screen_set_default_colors`.
- [ ] Add "boot up" or "connection" animations for the terminal window.
- [ ] Ensure compatibility with common CLI tools (`ls --color`, `top`, `vim`).

### Polish
- [ ] Mouse event forwarding (click, drag, scroll) for mouse-aware programs.
- [ ] Selection & clipboard (copy/paste).
- [ ] Debounce resize events to avoid excessive TIOCSWINSZ during window drag.
- [ ] Handle `SIGCHLD` to detect shell exit and show a message or restart.
- [ ] URL detection and click-to-open.
