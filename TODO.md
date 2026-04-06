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
- [x] Implement `TerminalModel` (QAbstractListModel):
    - [x] Map `libvterm` screen buffer to model rows.
    - [x] Expose cell data (character, foreground/background color, attributes).
    - [x] Efficiently signal partial updates (damage) to the view.
- [x] Add Unit Tests for `TerminalBackend` and `TerminalModel`.

## 4. Terminal Frontend (QML)
- [ ] Create `TerminalPanel.qml`.
- [ ] Implement grid-based text rendering using monospaced font (`Hack`).
- [ ] Implement keyboard interaction (handling `Keys.onPressed`).
- [ ] Add visual cursor (positioning and blinking).
- [ ] Handle terminal resizing when the window dimensions change.
- [ ] Map terminal cell colors to the Neo-Kitsch theme.

## 5. Styling & Integration
- [ ] Apply Neo-Kitsch theme colors from `Style.qml` to the terminal palette.
- [ ] Add "boot up" or "connection" animations for the terminal window.
- [ ] Ensure compatibility with common CLI tools like `ls`, `top`, and `vim`.
