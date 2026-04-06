import QtQuick
import TraceUI
import EncomGlobe

Item {
    id: globePanel
    implicitWidth: 800
    implicitHeight: 600
    clip: true

    property real globeRadius: 1.5
    property color baseColor: Style.accentGold
    property bool  introActive: false

    Globe {
        id: globe
        anchors.fill: parent
        anchors.margins: 10
        visible: globePanel.introActive
        
        dayLength: 60000  // slow rotation
        scale: 1.1
        viewAngle: 0.2
        
        baseColor: Style.accentGold
        pinColor: Style.accentSilver
        markerColor: Style.accentGold
        satelliteColor: Style.accentGold
        introLinesColor: Style.accentSilver
        markerSize: 0.2
        pinHeadSize: 0.3
        showLabels: false
        introDuration: AnimConfig.globeIntroDur
        startupDelay: 0
        
        Component.onCompleted: {
            // Add some decorative points
            addPin(40.7128, -74.0060, "NYC")
            addPin(51.5074, -0.1278, "LDN")
            addPin(35.6762, 139.6503, "TYO")
            addPin(-33.8688, 151.2093, "SYD")
            
            addSatellite(40.7128, -74.0060, 1.2)
            addSatellite(-33.8688, 151.2093, 1.3)
            
            addMarker(40.7128, -74.0060, "NYC", false)
            addMarker(34.0522, -118.2437, "LAX", true)
        }

        MouseArea {
            anchors.fill: parent
            property real lastX: 0
            property real lastY: 0
            
            onPressed: (mouse) => {
                lastX = mouse.x
                lastY = mouse.y
            }
            
            onPositionChanged: (mouse) => {
                if (pressed) {
                    let dx = mouse.x - lastX
                    globe.rotationOffset += dx * 0.005
                    let dy = mouse.y - lastY
                    globe.viewAngle = Math.max(-1.57, Math.min(1.57, 
                        globe.viewAngle + dy * 0.005))
                    lastX = mouse.x
                    lastY = mouse.y
                }
            }
            onWheel: (wheel) => {
                globe.scale = Math.max(0.3, Math.min(3.0, 
                    globe.scale + wheel.angleDelta.y * 0.001))
            }
        }

        Repeater {
            model: globe.pinLabels
            delegate: Text {
                visible: globe.showLabels
                x: modelData.x - width/2
                y: modelData.y - height
                text: modelData.text
                color: Style.textPrimary
                opacity: modelData.opacity
                font.family: Style.fontData
                font.pixelSize: 10
                font.bold: true
                style: Text.Outline
                styleColor: "black"
            }
        }
    }
}
