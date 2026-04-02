import QtQuick
import TraceUI

Window {
    id: root

    flags: Qt.FramelessWindowHint
    visibility: Window.FullScreen
    color: Style.backgroundColor
    visible: true

    FontLoader {
        source: "qrc:/fonts/BlenderPro-Bold.ttf"
    }
    FontLoader {
        source: "qrc:/fonts/BlenderPro-Book.ttf"
    }
    FontLoader {
        source: "qrc:/fonts/BlenderPro-Heavy.ttf"
    }
    FontLoader {
        source: "qrc:/fonts/BlenderPro-Medium.ttf"
    }
    FontLoader {
        source: "qrc:/fonts/BlenderPro-Thin.ttf"
    }
    FontLoader {
        source: "qrc:/fonts/Hack-Regular.ttf"
    }
    FontLoader {
        source: "qrc:/fonts/Hack-Bold.ttf"
    }
    FontLoader {
        source: "qrc:/fonts/Hack-Italic.ttf"
    }
    FontLoader {
        source: "qrc:/fonts/Hack-BoldItalic.ttf"
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
        width: 400
        height: 400
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
}
