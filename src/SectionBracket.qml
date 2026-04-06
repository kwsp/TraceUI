import QtQuick
import TraceUI

Item {
    id: root
    height: 10

    property alias text: labelText.text
    property bool showLabel: text !== ""

    // Horizontal line
    Rectangle {
        anchors.centerIn: parent
        width: parent.width
        height: 1
        color: Style.borderDefault
    }

    // Left vertical tick
    Rectangle {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: 1
        height: 10
        color: Style.borderDefault
    }

    // Right vertical tick
    Rectangle {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: 1
        height: 10
        color: Style.borderDefault
    }

    // Optional label in the middle
    Rectangle {
        visible: root.showLabel
        anchors.centerIn: parent
        width: labelText.width + 10
        height: labelText.height + 2
        color: Style.background

        Text {
            id: labelText
            anchors.centerIn: parent
            color: Style.textLabel
            font.family: Style.fontData
            font.pixelSize: Style.sizeHeader
            font.bold: true
        }
    }
}
