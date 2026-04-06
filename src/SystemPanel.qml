import QtQuick
import TraceUI

Item {
    id: root
    implicitWidth: 400
    implicitHeight: 700

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

    // Section 1: Header & Clock (top)
    Column {
        id: headerSection
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        anchors.topMargin: 25
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

        Grid {
            columns: 4
            width: parent.width
            spacing: 10

            Column {
                Text { text: "DATE"; color: Style.textLabel; font.family: Style.fontData; font.pixelSize: Style.sizeLabel }
                Text { text: Qt.formatDate(new Date(), "yyyy MMM dd").toUpperCase(); color: Style.textPrimary; font.family: Style.fontData; font.pixelSize: Style.sizeData }
            }

            Column {
                Text { text: "UPTIME"; color: Style.textLabel; font.family: Style.fontData; font.pixelSize: Style.sizeLabel }
                Text { text: SystemMonitor.uptime; color: Style.textPrimary; font.family: Style.fontData; font.pixelSize: Style.sizeData }
            }

            Column {
                Text { text: "CPU"; color: Style.textLabel; font.family: Style.fontData; font.pixelSize: Style.sizeLabel }
                Text { text: SystemMonitor.cpuName; color: Style.textPrimary; font.family: Style.fontData; font.pixelSize: Style.sizeData }
            }

            Column {
                Text { text: "OS"; color: Style.textLabel; font.family: Style.fontData; font.pixelSize: Style.sizeLabel }
                Text { text: SystemMonitor.osType; color: Style.textPrimary; font.family: Style.fontData; font.pixelSize: Style.sizeData }
            }
        }

    }

    // Section 2: CPU Usage
    Column {
        id: cpuSection
        anchors.top: headerSection.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        spacing: 5

        Row {
            width: parent.width
            Text {
                id: cpuUsageLabel
                text: "CPU USAGE"
                color: Style.textLabel
                font.family: Style.fontData
                font.pixelSize: Style.sizeHeader
                font.bold: true
            }
            Text {
                width: parent.width - cpuUsageLabel.width
                horizontalAlignment: Text.AlignRight
                text: SystemMonitor.cpuUsageTotal.toFixed(1) + "%"
                color: Style.accentGold
                font.family: Style.fontData
                font.pixelSize: Style.sizeData
            }
        }

        ShaderEffect {
            id: cpuShader
            width: parent.width
            height: 70
            property var  cpuHistory: cpuHistProvider
            property real graphScale: cpuHistProvider.graphScale
            property real phase:      cpuHistProvider.phase
            fragmentShader: "qrc:/shaders/cpugraph.frag.qsb"
        }

        Row {
            width: parent.width
            
            Column {
                width: parent.width / 4
                clip: true
                Text { text: "TEMP"; color: Style.textLabel; font.family: Style.fontData; font.pixelSize: Style.sizeLabel }
                Text { text: SystemMonitor.cpuTempCelsius.toFixed(1) + "°C"; color: Style.textPrimary; font.family: Style.fontData; font.pixelSize: Style.sizeData }
            }
            Column {
                width: parent.width / 4
                clip: true
                Text { text: "MIN"; color: Style.textLabel; font.family: Style.fontData; font.pixelSize: Style.sizeLabel }
                Text { 
                    width: parent.width
                    text: SystemMonitor.cpuClockMin.toFixed(2) + " GHz"
                    color: Style.textPrimary
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                }
            }
            Column {
                width: parent.width / 4
                clip: true
                Text { text: "MAX"; color: Style.textLabel; font.family: Style.fontData; font.pixelSize: Style.sizeLabel }
                Text { 
                    width: parent.width
                    text: SystemMonitor.cpuClockMax.toFixed(2) + " GHz"
                    color: Style.textPrimary
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                }
            }
            Column {
                width: parent.width / 4
                clip: true
                Text { text: "TASKS"; color: Style.textLabel; font.family: Style.fontData; font.pixelSize: Style.sizeLabel }
                Text { text: SystemMonitor.totalTasks.toString(); color: Style.textPrimary; font.family: Style.fontData; font.pixelSize: Style.sizeData }
            }
        }
    }

    // Section 3: Memory
    Column {
        id: memorySection
        anchors.top: cpuSection.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        spacing: 5

        Row {
            width: parent.width
            Text {
                id: memoryUsageLabel
                text: "MEMORY USAGE"
                color: Style.textLabel
                font.family: Style.fontData
                font.pixelSize: Style.sizeHeader
                font.bold: true
            }
            Text {
                width: parent.width - memoryUsageLabel.width
                horizontalAlignment: Text.AlignRight
                text: (SystemMonitor.ramUsedMB/1024).toFixed(1) + " / " + (SystemMonitor.ramTotalMB/1024).toFixed(1) + " GIB"
                color: Style.textPrimary
                font.family: Style.fontData
                font.pixelSize: Style.sizeData
            }
        }

        ShaderEffect {
            width: parent.width
            height: 50
            property real usageRatio: SystemMonitor.ramTotalMB > 0 ? SystemMonitor.ramUsedMB / SystemMonitor.ramTotalMB : 0.0
            fragmentShader: "qrc:/shaders/memorygrid.frag.qsb"
        }
    }

    // Section 4: Top Processes (fills remaining space)
    Column {
        id: processSection
        anchors.top: memorySection.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        anchors.bottomMargin: 15
        spacing: 5
        clip: true

        Text {
            text: "TOP PROCESSES"
            color: Style.textLabel
            font.family: Style.fontData
            font.pixelSize: Style.sizeHeader
            font.bold: true
        }

        ListView {
            id: processList
            width: parent.width
            height: parent.height - 25
            model: ProcessWatcher.processes
            clip: true
            interactive: true
            spacing: 2

            // Column widths - must match header and delegate
            readonly property int colPid: 40
            readonly property int colCpu: 45
            readonly property int colMem: 50
            readonly property int colSpacing: 1

            header: Row {
                id: headerRow
                width: processList.width
                height: 18
                spacing: processList.colSpacing
                Text { 
                    width: processList.colPid
                    text: "PID" 
                    color: Style.textLabel
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeLabel
                }
                Text { 
                    width: headerRow.width - processList.colPid - processList.colCpu - processList.colMem - (processList.colSpacing * 3)
                    text: "NAME" 
                    color: Style.textLabel
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeLabel
                }
                Text { 
                    id: cpuHeader
                    width: processList.colCpu
                    text: "CPU%" 
                    color: ProcessWatcher.sortByCpu ? Style.accentGold : Style.textLabel
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeLabel
                    horizontalAlignment: Text.AlignRight
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: ProcessWatcher.toggleSort()
                    }
                }
                Text { 
                    id: memHeader
                    width: processList.colMem
                    text: "MEM" 
                    color: ProcessWatcher.sortByCpu ? Style.textLabel : Style.accentGold
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeLabel
                    horizontalAlignment: Text.AlignRight
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: ProcessWatcher.toggleSort()
                    }
                }
            }

            delegate: Row {
                id: procRow
                required property int pid
                required property string name
                required property double cpuPct
                required property int ramMB

                width: processList.width
                height: 18
                spacing: processList.colSpacing
                
                Text { 
                    width: processList.colPid
                    text: procRow.pid.toString()
                    color: Style.accentSilver
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                }
                Text {
                    width: procRow.width - processList.colPid - processList.colCpu - processList.colMem - (processList.colSpacing * 3)
                    text: procRow.name
                    color: Style.textPrimary
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                    elide: Text.ElideRight
                }
                Text { 
                    width: processList.colCpu
                    text: procRow.cpuPct.toFixed(1)
                    color: Style.textPrimary
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                    horizontalAlignment: Text.AlignRight
                }
                Text { 
                    width: processList.colMem
                    text: procRow.ramMB + " MB"
                    color: Style.textPrimary
                    font.family: Style.fontData
                    font.pixelSize: Style.sizeData
                    horizontalAlignment: Text.AlignRight
                }
            }
        }
    }
}
