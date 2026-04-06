# TraceUI Terminal Implementation TODO

## 1. UI Restructuring
- [x] Shrink `GlobePanel` and position it next to `SystemPanel`.
- [x] Move `ProcessPanel` and `NetworkPanel` to maintain the left-hand status column.
- [x] Create a placeholder `TerminalPanel` occupying the right portion of the screen.

## 2. Terminal Backend (C++)
- [ ] Implement `TerminalBackend` class.
- [ ] Integrate POSIX `forkpty` for process and PTY management.
- [ ] Use `QSocketNotifier` for asynchronous reading from the PTY.
- [ ] Implement write functionality to send user input to the shell.
- [ ] Handle terminal resizing via `ioctl` (`TIOCSWINSZ`).

## 3. Terminal Emulation & Parsing
- [ ] Integrate a terminal escape sequence parser (e.g., `libvterm`).
- [ ] Maintain a character grid state (buffer) including text attributes (colors, styles).
- [ ] Implement a `TerminalModel` inheriting from `QAbstractListModel` to provide data to QML.

## 4. Terminal Frontend (QML)
- [ ] Create `TerminalPanel.qml`.
- [ ] Implement grid-based text rendering using monospaced font (`Hack`).
- [ ] Handle keyboard events and translate them to PTY escape sequences (arrow keys, etc.).
- [ ] Add cursor management (positioning and blinking).
- [ ] Implement basic scrollback functionality.

## 5. Styling & Integration
- [ ] Apply Neo-Kitsch theme colors from `Style.qml` to the terminal palette.
- [ ] Add "boot up" or "connection" animations for the terminal window.
- [ ] Ensure compatibility with common CLI tools like `ls`, `top`, and `vim`.
