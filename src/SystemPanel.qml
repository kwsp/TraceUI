import QtQuick
import TraceUI

Item {
    id: root
    width: 400
    height: 300

    property var cores: SystemMonitor.cpuUsagePercent || []

    // Simple historical accumulation for the shader (stores up to 64 normalized samples)
    property var historyData: new Array(64).fill(0.0)

    Connections {
        target: SystemMonitor
        function onDataUpdated() {
            let avg = SystemMonitor.cpuUsageTotal;

            // Shift history
            var temp = root.historyData;
            temp.shift();
            temp.push(avg / 100.0);
            root.historyData = temp;

            // Re-assign to force shader binding update
            cpuShader.history1 = Qt.vector4d(temp[0], temp[1], temp[2], temp[3]);
            // Just passing a few vectors for simplicity to the shader instead of a full texture,
            // or we can use QML Canvas if history gets too complex, but user requested shaders.
            // A more robust shader reads from a 1D texture, but for this exercise we'll pass
            // the latest reading and animate a procedural wave bounded by that reading.
            cpuShader.currentUsage = avg / 100.0;
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
            Text {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: SystemMonitor.cpuName
                color: Style.textLabel
                font.pixelSize: 10
                font.family: Style.fontData
            }
        }

        ShaderEffect {
            id: cpuShader
            anchors.top: cpuLabelRow.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.right: parent.right
            height: 50

            property real currentUsage: 0.1
            property vector4d history1: Qt.vector4d(0, 0, 0, 0)
            property real time

            NumberAnimation on time {
                loops: Animation.Infinite
                from: 0
                to: Math.PI * 2
                duration: 2000
            }

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
