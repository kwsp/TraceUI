import QtQuick
import TraceUI

Item {
    id: globePanel

    property real globeRadius: 1.5
    property color baseColor: Style.foregroundColor

    Image {
        id: landMaskImage
        source: "qrc:/textures/earth_landmask.png"
        visible: false
    }

    ShaderEffectSource {
        id: landMaskSource
        sourceItem: landMaskImage
        hideSource: true
    }

    ShaderEffect {
        anchors.fill: parent
        fragmentShader: "qrc:/shaders/globe.frag.qsb"

        property vector2d resolution: Qt.vector2d(globePanel.width, globePanel.height)
        property real time: 0.0
        property real globeRadius: globePanel.globeRadius
        property vector3d baseColor: Qt.vector3d(globePanel.baseColor.r, globePanel.baseColor.g, globePanel.baseColor.b)
        property var landMask: landMaskSource

        NumberAnimation on time {
            running: true
            loops: Animation.Infinite
            from: 0.0
            to: 36000.0
            duration: 36000000
        }
    }
}
