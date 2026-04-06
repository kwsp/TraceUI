import QtQuick
import TraceUI

FocusScope {
    id: root

    TerminalBackend {
        id: termBackend
    }

    TerminalModel {
        id: termModel
        backend: termBackend
    }

    Component.onCompleted: {
        terminalView.updateDimensions()
        termBackend.start()
        root.forceActiveFocus()
    }

    Rectangle {
        anchors.fill: parent
        color: Style.panelBg
        border.color: root.activeFocus ? Style.accentGold : Style.borderDefault
        border.width: 1

        MouseArea {
            anchors.fill: parent
            onClicked: root.forceActiveFocus()
        }

        ListView {
            id: terminalView
            anchors.fill: parent
            anchors.margins: 10
            model: termModel
            clip: true
            interactive: false // Terminal handles scrolling internally usually

            onWidthChanged: updateDimensions()
            onHeightChanged: updateDimensions()

            function updateDimensions() {
                if (width > 0 && height > 0) {
                    termBackend.cols = Math.floor(width / 9)
                    termBackend.rows = Math.floor(height / 18)
                }
            }

            delegate: Text {
                width: terminalView.width
                height: 18
                text: model.text
                color: Style.textPrimary
                font.family: Style.fontData
                font.pixelSize: 14
                verticalAlignment: Text.AlignVCenter
            }

            Connections {
                target: termBackend
                function onCursorMoved() {
                    let rowY = termBackend.cursorRow * 18
                    if (rowY < terminalView.contentY) {
                        terminalView.contentY = rowY
                    } else if (rowY + 18 > terminalView.contentY + terminalView.height) {
                        terminalView.contentY = rowY + 18 - terminalView.height
                    }
                }
            }
        }

        Rectangle {
            id: cursor
            width: 9
            height: 18
            color: Style.accentGold
            x: 10 + termBackend.cursorCol * 9
            y: 10 + termBackend.cursorRow * 18 - terminalView.contentY
            visible: root.activeFocus && (y >= 10 && y <= terminalView.height + 10)

            SequentialAnimation on opacity {
                loops: Animation.Infinite
                NumberAnimation { from: 1.0; to: 0.0; duration: 500 }
                NumberAnimation { from: 0.0; to: 1.0; duration: 500 }
            }
        }
    }

    // Keyboard handling
    focus: true
    Keys.onPressed: (event) => {
        console.log("Key pressed:", event.key, "text:", event.text);
        if (event.modifiers & Qt.ControlModifier) {
            if (event.key >= Qt.Key_A && event.key <= Qt.Key_Z) {
                // Ctrl+A is 1, Ctrl+B is 2, etc.
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
        } else if (event.text.length > 0) {
            termBackend.sendInput(event.text)
        } else {
            return;
        }
        event.accepted = true
    }
}
