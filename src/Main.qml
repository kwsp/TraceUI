import QtQuick
import TraceUI

Window {
    id: root

    property bool buildTerminal: false

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

    // Terminal Panel (Right 2/3)
    Loader {
        id: terminalLoader
        anchors.left: globePanel.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 24
        focus: true
        active: root.buildTerminal
        // Absolute QRC path to the module component
        source: root.buildTerminal ? "qrc:/qt/qml/TraceUI/Terminal/TerminalPanel.qml" : ""

        onLoaded: console.log("Terminal panel loaded successfully")
        onStatusChanged: {
            if (status === Loader.Error) {
                console.error("Terminal Loader Error:", source, errorString())
            }
        }
    }
}
