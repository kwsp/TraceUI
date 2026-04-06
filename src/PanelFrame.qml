import QtQuick
import TraceUI

Item {
    id: root
    anchors.fill: parent

    property string title: ""
    property bool showBrackets: true

    // Top bracket
    Rectangle {
        id: topBracket
        visible: root.showBrackets
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: Style.borderDefault

        // Top-left vertical tick
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            width: 1
            height: 10
            color: Style.borderDefault
        }

        // Top-right vertical tick
        Rectangle {
            anchors.top: parent.top
            anchors.right: parent.right
            width: 1
            height: 10
            color: Style.borderDefault
        }

        // Title Label
        Rectangle {
            id: labelBg
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 20
            width: titleText.width + 10
            height: titleText.height + 2
            color: Style.background

            Text {
                id: titleText
                anchors.centerIn: parent
                text: root.title
                color: Style.textLabel
                font.family: Style.fontData
                font.pixelSize: Style.sizeLabel
                font.bold: true
            }
        }
    }

    // Bottom bracket
    Rectangle {
        id: bottomBracket
        visible: root.showBrackets
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: Style.borderDefault

        // Bottom-left vertical tick
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: 1
            height: 10
            color: Style.borderDefault
        }

        // Bottom-right vertical tick
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: 1
            height: 10
            color: Style.borderDefault
        }
    }
}
