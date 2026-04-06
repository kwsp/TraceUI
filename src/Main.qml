import QtQuick
import TraceUI

Window {
    id: root

    flags: Qt.FramelessWindowHint
    visibility: Window.FullScreen
    color: Style.backgroundColor
    visible: true

    FontLoader {
        source: "qrc:/fonts/BlenderPro-Book.ttf"
    }
    FontLoader {
        source: "qrc:/fonts/Hack-Regular.ttf"
    }

    // Dot grid background
    ShaderEffect {
        anchors.fill: parent
        fragmentShader: "qrc:/shaders/dotgrid.frag.qsb"

        property vector2d resolution: Qt.vector2d(width, height)
        property real spacing: 20.0
        property real radius: 0.5
        property real dotAlpha: 0.2
    }

    SystemPanel {
        id: sysPanel
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 24
    }

    GlobePanel {
        id: globePanel
        anchors.left: sysPanel.right
        anchors.top: sysPanel.top
        width: 300 // Shrunk
        height: 300
        anchors.margins: 24
    }

    ProcessPanel {
        id: procPanel
        anchors.top: sysPanel.bottom
        anchors.left: parent.left
        anchors.margins: 24
        anchors.topMargin: 24
    }

    NetworkPanel {
        id: netPanel
        anchors.top: procPanel.bottom
        anchors.left: parent.left
        anchors.margins: 24
        anchors.topMargin: 24
    }

    // Terminal Placeholder (Right 2/3)
    Rectangle {
        id: terminalPlaceholder
        anchors.left: globePanel.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 24
        color: Qt.rgba(Style.accentGold.r, Style.accentGold.g, Style.accentGold.b, 0.05)
        border.color: Style.borderDefault
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: "TERMINAL SYSTEM OFFLINE"
            color: Style.textDim
            font.family: Style.fontDisplay
            font.pixelSize: 32
        }
    }
}
