import QtQuick
import TraceUI

Item {
    id: root
    width: 400
    height: 700

    CpuHistoryProvider {
        id: cpuHistProvider
        visible: false
    }

    Connections {
        target: SystemMonitor
        function onDataUpdated() {
            cpuHistProvider.onDataUpdated(SystemMonitor.cpuUsageUser, SystemMonitor.cpuUsageSystem);
        }
    }

    PanelFrame {
        anchors.fill: parent
        title: "SYSTEM"
    }

    Column {
        anchors.fill: parent
        anchors.margins: 15
        anchors.topMargin: 25 // Make room for top bracket/title
        spacing: 20

        // 1. Header & Clock
        Column {
            width: parent.width
            spacing: 5

            Text {
                id: clockText
                text: Qt.formatTime(new Date(), "hh:mm:ss")
                color: Style.textPrimary
                font.family: Style.fontDisplay
                font.pixelSize: Style.sizeClock
                
                Timer {
                    interval: 1000; running: true; repeat: true
                    onTriggered: clockText.text = Qt.formatTime(new Date(), "hh:mm:ss")
                }
            }

            Row {
                spacing: 15
                Text {
                    text: Qt.formatDate(new Date(), "yyyy MMM dd").toUpperCase()
                    color: Style.textLabel
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                }
                Text {
                    text: "UPTIME " + SystemMonitor.uptime
                    color: Style.textLabel
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                }
                Text {
                    text: SystemMonitor.osType
                    color: Style.textLabel
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                }
                Text {
                    text: SystemMonitor.powerSource
                    color: Style.accentGold
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                }
            }
        }

        // 2. CPU Usage
        Column {
            width: parent.width
            spacing: 8

            Text {
                text: "CPU USAGE / " + SystemMonitor.cpuName
                color: Style.textLabel
                font.family: Style.fontData
                font.pixelSize: Style.sizeHeader
                font.bold: true
            }

            ShaderEffect {
                id: cpuShader
                width: parent.width
                height: 80
                property var  cpuHistory: cpuHistProvider
                property real graphScale: cpuHistProvider.graphScale
                property real phase:      cpuHistProvider.phase
                fragmentShader: "qrc:/shaders/cpugraph.frag.qsb"
            }

            Grid {
                columns: 3
                width: parent.width
                spacing: 10
                
                Column {
                    Text { text: "TEMP"; color: Style.textLabel; font.pixelSize: Style.sizeLabel; font.family: Style.fontData }
                    Text { text: SystemMonitor.cpuTempCelsius.toFixed(1) + "°C"; color: Style.textPrimary; font.pixelSize: Style.sizeData; font.family: Style.fontData }
                }
                Column {
                    Text { text: "CLOCK (MIN/MAX)"; color: Style.textLabel; font.pixelSize: Style.sizeLabel; font.family: Style.fontData }
                    Text { text: SystemMonitor.cpuClockMin.toFixed(2) + " / " + SystemMonitor.cpuClockMax.toFixed(2) + " GHz"; color: Style.textPrimary; font.pixelSize: Style.sizeData; font.family: Style.fontData }
                }
                Column {
                    Text { text: "TASKS"; color: Style.textLabel; font.pixelSize: Style.sizeLabel; font.family: Style.fontData }
                    Text { text: SystemMonitor.totalTasks.toString(); color: Style.textPrimary; font.pixelSize: Style.sizeData; font.family: Style.fontData }
                }
            }
        }

        // 3. Memory
        Column {
            width: parent.width
            spacing: 8

            Text {
                text: "USING " + (SystemMonitor.ramUsedMB/1024).toFixed(1) + " OUT OF " + (SystemMonitor.ramTotalMB/1024).toFixed(1) + " GIB"
                color: Style.textLabel
                font.family: Style.fontData
                font.pixelSize: Style.sizeHeader
                font.bold: true
            }

            ShaderEffect {
                width: parent.width
                height: 60
                property real usageRatio: SystemMonitor.ramTotalMB > 0 ? SystemMonitor.ramUsedMB / SystemMonitor.ramTotalMB : 0.0
                fragmentShader: "qrc:/shaders/memorygrid.frag.qsb"
            }
        }

        // 4. Top Processes
        Column {
            width: parent.width
            height: 200
            spacing: 8

            Text {
                text: "TOP PROCESSES"
                color: Style.textLabel
                font.family: Style.fontData
                font.pixelSize: Style.sizeHeader
                font.bold: true
            }

            ListView {
                width: parent.width
                height: parent.height - 30
                model: ProcessWatcher.processes
                clip: true
                interactive: false

                header: Row {
                    width: parent.width
                    height: 20
                    Text { width: 60; text: "PID"; color: Style.textLabel; font.pixelSize: Style.sizeLabel; font.family: Style.fontData }
                    Text { width: 140; text: "NAME"; color: Style.textLabel; font.pixelSize: Style.sizeLabel; font.family: Style.fontData }
                    Text { width: 60; text: "CPU%"; color: Style.textLabel; font.pixelSize: Style.sizeLabel; font.family: Style.fontData; horizontalAlignment: Text.AlignRight }
                    Text { width: 60; text: "MEM"; color: Style.textLabel; font.pixelSize: Style.sizeLabel; font.family: Style.fontData; horizontalAlignment: Text.AlignRight }
                }

                delegate: Row {
                    id: procRow
                    required property int pid
                    required property string name
                    required property double cpuPct
                    required property int ramMB

                    width: parent.width
                    height: 20
                    
                    Text { width: 60; text: procRow.pid.toString(); color: Style.accentSilver; font.pixelSize: Style.sizeData; font.family: Style.fontData }
                    Text { width: 140; text: procRow.name; color: Style.textPrimary; font.pixelSize: Style.sizeData; font.family: Style.fontData; elide: Text.ElideRight }
                    Text { width: 60; text: procRow.cpuPct.toFixed(1); color: Style.accentGold; font.pixelSize: Style.sizeData; font.family: Style.fontData; horizontalAlignment: Text.AlignRight }
                    Text { width: 60; text: procRow.ramMB.toString(); color: Style.textPrimary; font.pixelSize: Style.sizeData; font.family: Style.fontData; horizontalAlignment: Text.AlignRight }
                }
            }
        }
    }
}
