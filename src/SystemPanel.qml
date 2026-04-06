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
                LabelText { text: "DATE" }
                DataText { text: Qt.formatDate(new Date(), "yyyy MMM dd").toUpperCase() }
            }

            Column {
                LabelText { text: "UPTIME" }
                DataText { text: SystemMonitor.uptime }
            }

            Column {
                LabelText { text: "CPU" }
                DataText { text: SystemMonitor.cpuName }
            }

            Column {
                LabelText { text: "OS" }
                DataText { text: SystemMonitor.osType }
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
            HeaderText {
                id: cpuUsageLabel
                text: "CPU USAGE"
            }
            DataText {
                width: parent.width - cpuUsageLabel.width
                horizontalAlignment: Text.AlignRight
                text: SystemMonitor.cpuUsageTotal.toFixed(1) + "%"
                color: Style.accentGold
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
                LabelText { text: "TEMP" }
                DataText { text: SystemMonitor.cpuTempCelsius.toFixed(1) + "°C" }
            }
            Column {
                width: parent.width / 4
                clip: true
                LabelText { text: "MIN" }
                DataText { 
                    width: parent.width
                    text: SystemMonitor.cpuClockMin.toFixed(2) + " GHz"
                }
            }
            Column {
                width: parent.width / 4
                clip: true
                LabelText { text: "MAX" }
                DataText { 
                    width: parent.width
                    text: SystemMonitor.cpuClockMax.toFixed(2) + " GHz"
                }
            }
            Column {
                width: parent.width / 4
                clip: true
                LabelText { text: "TASKS" }
                DataText { text: SystemMonitor.totalTasks.toString() }
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
            HeaderText {
                id: memoryUsageLabel
                text: "MEMORY USAGE"
            }
            DataText {
                width: parent.width - memoryUsageLabel.width
                horizontalAlignment: Text.AlignRight
                text: (SystemMonitor.ramUsedMB/1024).toFixed(1) + " / " + (SystemMonitor.ramTotalMB/1024).toFixed(1) + " GIB"
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

        HeaderText {
            text: "TOP PROCESSES"
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
                LabelText { 
                    width: processList.colPid
                    text: "PID" 
                }
                LabelText { 
                    width: headerRow.width - processList.colPid - processList.colCpu - processList.colMem - (processList.colSpacing * 3)
                    text: "NAME" 
                }
                LabelText { 
                    id: cpuHeader
                    width: processList.colCpu
                    text: "CPU%" 
                    color: ProcessWatcher.sortByCpu ? Style.accentGold : Style.textLabel
                    horizontalAlignment: Text.AlignRight
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: ProcessWatcher.toggleSort()
                    }
                }
                LabelText { 
                    id: memHeader
                    width: processList.colMem
                    text: "MEM" 
                    color: ProcessWatcher.sortByCpu ? Style.textLabel : Style.accentGold
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
                
                DataText { 
                    width: processList.colPid
                    text: procRow.pid.toString()
                    color: Style.accentSilver
                }
                DataText {
                    width: procRow.width - processList.colPid - processList.colCpu - processList.colMem - (processList.colSpacing * 3)
                    text: procRow.name
                    elide: Text.ElideRight
                }
                DataText { 
                    width: processList.colCpu
                    text: procRow.cpuPct.toFixed(1)
                    horizontalAlignment: Text.AlignRight
                }
                DataText { 
                    width: processList.colMem
                    text: procRow.ramMB + " MB"
                    horizontalAlignment: Text.AlignRight
                }
            }
        }
    }
}
