import QtQuick
import TraceUI

Item {
    id: root
    width: 400
    height: 300

    CpuHistoryProvider {
        id: cpuHistProvider
        visible: false
        width: 1
        height: 1
    }

    Connections {
        target: SystemMonitor
        function onDataUpdated() {
            cpuHistProvider.onDataUpdated(SystemMonitor.cpuUsageUser, SystemMonitor.cpuUsageSystem);
        }
    }

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
            text: "SYSTEM STATUS"
            color: Style.textPrimary
            font.pixelSize: 12
            font.family: Style.fontData
        }

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Style.borderDefault
        }
    }

    // Top Clock Section
    Item {
        id: timeSection
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 60

        Text {
            id: timeText
            anchors.centerIn: parent
            text: Qt.formatTime(new Date(), "hh:mm:ss")
            color: Style.textPrimary
            font.family: Style.fontDisplay
            font.pixelSize: 48
        }

        Timer {
            interval: 1000
            running: true
            repeat: true
            onTriggered: timeText.text = Qt.formatTime(new Date(), "hh:mm:ss")
        }
    }

    // System Info
    Item {
        id: sysInfoSection
        anchors.top: timeSection.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 60

        Row {
            anchors.centerIn: parent
            spacing: 20

            Column {
                Text {
                    text: "UPTIME"
                    color: Style.textLabel
                    font.pixelSize: 10
                    font.family: Style.fontData
                }
                Text {
                    text: SystemMonitor.uptime
                    color: Style.textPrimary
                    font.pixelSize: 14
                    font.family: Style.fontData
                }
            }
            Column {
                Text {
                    text: "LOAD"
                    color: Style.textLabel
                    font.pixelSize: 10
                    font.family: Style.fontData
                }
                Text {
                    text: SystemMonitor.loadAverage1m.toFixed(2)
                    color: Style.textPrimary
                    font.pixelSize: 14
                    font.family: Style.fontData
                }
            }
            Column {
                Text {
                    text: "TEMP"
                    color: Style.textLabel
                    font.pixelSize: 10
                    font.family: Style.fontData
                }
                Text {
                    text: SystemMonitor.cpuTempCelsius.toFixed(1) + "°C"
                    color: Style.textPrimary
                    font.pixelSize: 14
                    font.family: Style.fontData
                }
            }
        }
    }

    // CPU & Memory Shaders
    Item {
        anchors.top: sysInfoSection.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10

        Item {
            id: cpuLabelRow
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 20

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: "CPU USAGE"
                color: Style.textLabel
                font.pixelSize: 10
                font.family: Style.fontData
            }
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 15

                Text {
                    text: "USR: " + SystemMonitor.cpuUsageUser.toFixed(1) + "%"
                    color: Style.accentGold
                    font.pixelSize: 10
                    font.family: Style.fontData
                }
                Text {
                    text: "SYS: " + SystemMonitor.cpuUsageSystem.toFixed(1) + "%"
                    color: Style.accentSilver
                    font.pixelSize: 10
                    font.family: Style.fontData
                }
            }
        }

        ShaderEffect {
            id: cpuShader
            anchors.top: cpuLabelRow.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.right: parent.right
            height: 50

            property var  cpuHistory: cpuHistProvider
            property real graphScale: cpuHistProvider.graphScale
            property real phase:      cpuHistProvider.phase

            fragmentShader: "qrc:/shaders/cpugraph.frag.qsb"
        }

        Item {
            id: memLabelRow
            anchors.top: cpuShader.bottom
            anchors.topMargin: 10
            anchors.left: parent.left
            anchors.right: parent.right
            height: 20

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: "MEMORY"
                color: Style.textLabel
                font.pixelSize: 10
                font.family: Style.fontData
            }
            Text {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: (SystemMonitor.ramUsedMB / 1024).toFixed(1) + " GB / " + (SystemMonitor.ramTotalMB / 1024).toFixed(1) + " GB"
                color: Style.textLabel
                font.pixelSize: 10
                font.family: Style.fontData
            }
        }

        ShaderEffect {
            id: memShader
            anchors.top: memLabelRow.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            property real usageRatio: SystemMonitor.ramTotalMB > 0 ? SystemMonitor.ramUsedMB / SystemMonitor.ramTotalMB : 0.0
            fragmentShader: "qrc:/shaders/memorygrid.frag.qsb"
        }
    }
}
