import QtQuick

Window {
    id: root

    flags: Qt.FramelessWindowHint
    visibility: Window.FullScreen
    color: "#0d0b0e"
    visible: true

    // Dot grid background
    ShaderEffect {
        anchors.fill: parent
        fragmentShader: "qrc:/shaders/dotgrid.frag.qsb"

        property vector2d resolution: Qt.vector2d(width, height)
        property real spacing: 20.0
        property real radius: 0.5
        property real dotAlpha: 0.15
    }
}
