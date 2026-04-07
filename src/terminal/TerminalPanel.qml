import QtQuick
import TraceUI
import TraceUITerminal

FocusScope {
    id: root

    property alias backend: termBackend

    TerminalBackend {
        id: termBackend
    }

    Component.onCompleted: {
        renderer.updateGridSize()
        termBackend.start()
        root.forceActiveFocus()
    }

    Rectangle {
        anchors.fill: parent
        color: Style.backgroundColor
        border.color: root.activeFocus ? Style.accentGold : Style.borderDefault
        border.width: 1

        MouseArea {
            anchors.fill: parent
            onClicked: root.forceActiveFocus()
        }

        TerminalRenderer {
            id: renderer
            anchors.fill: parent
            anchors.margins: 4
            backend: termBackend
            fontFamily: Style.fontData
            fontSize: 14

            onCellMetricsChanged: updateGridSize()
            onWidthChanged: updateGridSize()
            onHeightChanged: updateGridSize()

            function updateGridSize() {
                if (width > 0 && height > 0 && cellWidth > 0 && cellHeight > 0) {
                    let newCols = Math.floor(width / cellWidth)
                    let newRows = Math.floor(height / cellHeight)
                    if (newCols > 0 && newRows > 0) {
                        termBackend.resize(newRows, newCols)
                    }
                }
            }
        }

        // Blinking cursor overlay
        Rectangle {
            id: cursor
            width: renderer.cellWidth
            height: renderer.cellHeight
            color: Style.accentGold
            opacity: cursorBlink.running ? cursorOpacity : 1.0
            x: renderer.x + termBackend.cursorCol * renderer.cellWidth
            y: renderer.y + termBackend.cursorRow * renderer.cellHeight
            visible: root.activeFocus
                     && y >= renderer.y
                     && y + height <= renderer.y + renderer.height

            property real cursorOpacity: 1.0
            SequentialAnimation {
                id: cursorBlink
                loops: Animation.Infinite
                running: root.activeFocus
                NumberAnimation { target: cursor; property: "cursorOpacity"; from: 1.0; to: 0.0; duration: 500 }
                NumberAnimation { target: cursor; property: "cursorOpacity"; from: 0.0; to: 1.0; duration: 500 }
            }
        }
    }

    // Keyboard handling
    focus: true
    Keys.onPressed: (event) => {
        if (event.modifiers & Qt.ControlModifier) {
            if (event.key >= Qt.Key_A && event.key <= Qt.Key_Z) {
                let ctrlKey = String.fromCharCode(event.key - Qt.Key_A + 1)
                termBackend.sendInput(ctrlKey)
                event.accepted = true
                return
            }
        }

        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            termBackend.sendInput("\r")
        } else if (event.key === Qt.Key_Backspace) {
            termBackend.sendInput("\x7f")
        } else if (event.key === Qt.Key_Tab) {
            termBackend.sendInput("\t")
        } else if (event.key === Qt.Key_Escape) {
            termBackend.sendInput("\x1b")
        } else if (event.key === Qt.Key_Up) {
            termBackend.sendInput("\x1b[A")
        } else if (event.key === Qt.Key_Down) {
            termBackend.sendInput("\x1b[B")
        } else if (event.key === Qt.Key_Right) {
            termBackend.sendInput("\x1b[C")
        } else if (event.key === Qt.Key_Left) {
            termBackend.sendInput("\x1b[D")
        } else if (event.key === Qt.Key_Home) {
            termBackend.sendInput("\x1b[H")
        } else if (event.key === Qt.Key_End) {
            termBackend.sendInput("\x1b[F")
        } else if (event.key === Qt.Key_Delete) {
            termBackend.sendInput("\x1b[3~")
        } else if (event.key === Qt.Key_PageUp) {
            termBackend.sendInput("\x1b[5~")
        } else if (event.key === Qt.Key_PageDown) {
            termBackend.sendInput("\x1b[6~")
        } else if (event.text.length > 0) {
            termBackend.sendInput(event.text)
        } else {
            return
        }
        event.accepted = true
    }
}
