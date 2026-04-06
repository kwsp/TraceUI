import QtQuick
import TraceUI
import EncomGlobe

Item {
    id: root
    width: 400
    height: 700

    NetworkHistoryProvider {
        id: netHistProvider
        visible: false
    }

    Connections {
        target: NetworkMonitor
        function onDataUpdated() {
            netHistProvider.onDataUpdated(NetworkMonitor.downloadBytesPerSec, NetworkMonitor.uploadBytesPerSec);
        }
    }

    PanelFrame {
        title: "NETWORK"
    }

    Column {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 20

        // 1. Network Status
        Column {
            width: parent.width
            spacing: 5

            Text {
                text: "NETWORK STATUS"
                color: Style.textLabel
                font.family: Style.fontData
                font.pixelSize: Style.sizeHeader
                font.bold: true
            }

            Grid {
                columns: 2
                width: parent.width
                spacing: 10
                
                Column {
                    Text { text: "IPV4 ADDRESS"; color: Style.textLabel; font.pixelSize: Style.sizeLabel; font.family: Style.fontData }
                    Text { text: NetworkMonitor.ipv4Address; color: Style.textPrimary; font.pixelSize: Style.sizeData; font.family: Style.fontData }
                }
                Column {
                    Text { text: "STATE / ONLINE"; color: Style.textLabel; font.pixelSize: Style.sizeLabel; font.family: Style.fontData }
                    Text { text: NetworkMonitor.isOnline ? "ONLINE" : "OFFLINE"; color: NetworkMonitor.isOnline ? Style.accentGold : Style.accentError; font.pixelSize: Style.sizeData; font.family: Style.fontData }
                }
                Column {
                    Text { text: "PING RESPONSE"; color: Style.textLabel; font.pixelSize: Style.sizeLabel; font.family: Style.fontData }
                    Text { text: NetworkMonitor.pingMs.toString() + " MS"; color: Style.textPrimary; font.pixelSize: Style.sizeData; font.family: Style.fontData }
                }
                Column {
                    Text { text: "PACKET LOSS"; color: Style.textLabel; font.pixelSize: Style.sizeLabel; font.family: Style.fontData }
                    Text { text: (NetworkMonitor.packetLossPct * 100).toFixed(1) + " %"; color: Style.textPrimary; font.pixelSize: Style.sizeData; font.family: Style.fontData }
                }
            }
        }

        // 2. World View
        Column {
            width: parent.width
            spacing: 5
            height: 350

            Text {
                text: "GLOBAL NETWORK MAP / ENDPOINT: 41.8781, -87.6298"
                color: Style.textLabel
                font.family: Style.fontData
                font.pixelSize: Style.sizeHeader
                font.bold: true
            }

            GlobePanel {
                width: parent.width
                height: 320
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        // 3. Network Traffic
        Column {
            width: parent.width
            spacing: 8

            Text {
                text: "TOTAL TRAFFIC FLOW"
                color: Style.textLabel
                font.family: Style.fontData
                font.pixelSize: Style.sizeHeader
                font.bold: true
            }

            Row {
                spacing: 20
                Text {
                    text: (NetworkMonitor.downloadBytesPerSec / 1024.0 / 1024.0).toFixed(1) + " MB OUT"
                    color: Style.accentGold
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                }
                Text {
                    text: (NetworkMonitor.uploadBytesPerSec / 1024.0 / 1024.0).toFixed(1) + " MB IN"
                    color: Style.accentSilver
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                }
            }

            Item {
                width: parent.width
                height: 100
                
                // Y-axis labels
                Column {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 30
                    z: 10
                    Text { text: " 0.61"; color: Style.textLabel; font.pixelSize: 8; font.family: Style.fontData }
                    Item { height: 70; width: 1 }
                    Text { text: "-1.61"; color: Style.textLabel; font.pixelSize: 8; font.family: Style.fontData; anchors.bottom: parent.bottom }
                }

                ShaderEffect {
                    anchors.fill: parent
                    anchors.leftMargin: 30
                    property var  networkHistory: netHistProvider
                    property real graphScale:     netHistProvider.graphScale
                    property real phase:          netHistProvider.phase
                    fragmentShader: "qrc:/shaders/networkgraph.frag.qsb"
                }
            }
        }
    }
}
