import QtQuick
import TraceUI

Item {
    id: root
    width: 400
    height: 300

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
            text: "TOP PROCESSES"
            color: Style.textPrimary
            font.pixelSize: 12
            font.family: Style.fontData
        }

        Row {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 10
            spacing: 6

            Text {
                text: "CPU%"
                color: processWatcher.sortByCpu ? Style.accentGold : Style.textLabel
                font.pixelSize: 10
                font.family: Style.fontData
            }
            Text {
                text: "|"
                color: Style.textDim
                font.pixelSize: 10
                font.family: Style.fontData
            }
            Text {
                text: "RAM MB"
                color: processWatcher.sortByCpu ? Style.textLabel : Style.accentGold
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

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: true
        onClicked: mouse => {
            processWatcher.toggleSort();
            mouse.accepted = false;
        }
    }

    ListView {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        clip: true

        model: processWatcher.processes

        delegate: Item {
            id: delegateRoot
            required property int pid
            required property string name
            required property int cpuPct
            required property int ramMB

            width: ListView.view.width
            height: 20

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: delegateRoot.pid.toString().padEnd(6, " ") + " " + delegateRoot.name
                color: Style.accentSilver
                font.pixelSize: 12
                font.family: Style.fontData
            }

            Text {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: delegateRoot.cpuPct + "%  " + delegateRoot.ramMB + " MB"
                color: Style.textPrimary
                font.pixelSize: 12
                font.family: Style.fontData
            }
        }
    }
}
