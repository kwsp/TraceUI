import QtQuick
import TraceUI
import EncomGlobe

Item {
    id: root
    implicitWidth: 400
    implicitHeight: 700

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
        anchors.fill: parent
        title: "NETWORK"
    }

    // Section 1: Network Status (top)
    Column {
        id: statusSection
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        anchors.topMargin: 30
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

    // Bracket separator between status and globe
    SectionBracket {
        id: divider1
        anchors.top: statusSection.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 8
        text: "GLOBAL NETWORK MAP"
    }

    // Section 2: World View (center, fills remaining space)
    Column {
        id: globeSection
        anchors.top: divider1.bottom
        anchors.bottom: divider2.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        spacing: 5

        Text {
            text: "ENDPOINT: 41.8781, -87.6298"
            color: Style.textPrimary
            font.family: Style.fontData
            font.pixelSize: Style.sizeData
        }

        GlobePanel {
            width: parent.width
            height: parent.height - 25
        }
    }

    // Bracket separator between globe and traffic
    SectionBracket {
        id: divider2
        anchors.bottom: trafficSection.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.bottomMargin: 8
        text: "TOTAL TRAFFIC FLOW"
    }

    // Section 3: Traffic Flow (bottom)
    Column {
        id: trafficSection
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        anchors.bottomMargin: 15
        spacing: 5

        Row {
            spacing: 15
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
            height: 70
            
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
