import QtQuick
import TraceUI

Item {
    id: root
    width: 400
    height: 200

    Rectangle {
        anchors.fill: parent
        color: Style.panelBg
        border.color: Style.borderDefault
        border.width: 1
    }
    
    Item {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        
        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 10
            text: "NETWORK STATUS"
            color: Style.textPrimary
            font.pixelSize: 12
            font.family: Style.fontData
        }
        
        Row {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 10
            spacing: 20
            
            Text {
                text: "IFACE " + NetworkMonitor.mainInterface
                color: Style.textDim
                font.pixelSize: 10
                font.family: Style.fontData
            }
            Text {
                text: NetworkMonitor.vpnActive ? "VPN SECURE" : "NET ONLINE"
                color: NetworkMonitor.vpnActive ? Style.accentGold : Style.textPrimary
                font.pixelSize: 10
                font.family: Style.fontData
            }
        }
        
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Style.borderDefault
        }
    }
    
    Item {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        
        Item {
            id: labelNetRow
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 20
            
            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: "NETWORK TRAFFIC"
                color: Style.textLabel
                font.pixelSize: 10
                font.family: Style.fontData
            }
            Text {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: "DL: " + (NetworkMonitor.downloadBytesPerSec / 1024.0).toFixed(1) + " KB/s  UL: " + (NetworkMonitor.uploadBytesPerSec / 1024.0).toFixed(1) + " KB/s"
                color: Style.textLabel
                font.pixelSize: 10
                font.family: Style.fontData
            }
        }
        
        ShaderEffect {
            id: netShader
            anchors.top: labelNetRow.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 5
            
            property real dlSignal: Math.min(NetworkMonitor.downloadBytesPerSec / 102400.0, 1.0)
            property real ulSignal: Math.min(NetworkMonitor.uploadBytesPerSec / 102400.0, 1.0)
            property real time
            
            NumberAnimation on time { loops: Animation.Infinite; from: 0; to: 10000.0; duration: 10000000 }
            
            fragmentShader: "qrc:/shaders/networkgraph.frag.qsb"
        }
    }
}
