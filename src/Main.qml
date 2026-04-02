import QtQuick

Window {
    id: root

    flags: Qt.FramelessWindowHint
    visibility: Window.FullScreen
    color: "#0d0b0e"
    visible: true

    FontLoader { source: "qrc:/fonts/BlenderPro-Bold.ttf" }
    FontLoader { source: "qrc:/fonts/BlenderPro-Book.ttf" }
    FontLoader { source: "qrc:/fonts/BlenderPro-Heavy.ttf" }
    FontLoader { source: "qrc:/fonts/BlenderPro-Medium.ttf" }
    FontLoader { source: "qrc:/fonts/BlenderPro-Thin.ttf" }
    FontLoader { source: "qrc:/fonts/Hack-Regular.ttf" }
    FontLoader { source: "qrc:/fonts/Hack-Bold.ttf" }
    FontLoader { source: "qrc:/fonts/Hack-Italic.ttf" }
    FontLoader { source: "qrc:/fonts/Hack-BoldItalic.ttf" }

    // Dot grid background
    ShaderEffect {
        anchors.fill: parent
        fragmentShader: "qrc:/shaders/dotgrid.frag.qsb"

        property vector2d resolution: Qt.vector2d(width, height)
        property real spacing: 20.0
        property real radius: 0.5
        property real dotAlpha: 0.2
    }

    ClockPanel {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 24
    }
}
