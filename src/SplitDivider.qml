import QtQuick
import QtQuick.Controls
import TraceUI

// Custom handle for SplitView — a thin coloured line that glows on hover.
Rectangle {
    id: root

    // True for horizontal SplitView (handle is vertical bar), false for vertical.
    property bool vertical: true

    implicitWidth:  vertical ? 8 : parent.width
    implicitHeight: vertical ? parent.height : 8

    color: "transparent"

    // The visible line
    Rectangle {
        anchors.centerIn: parent
        width:  root.vertical ? 1 : parent.width
        height: root.vertical ? parent.height : 1
        color: hoverHandler.hovered || SplitHandle.pressed
               ? Style.accentGold
               : Style.borderDefault

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }

    // Wider invisible hit area so the handle is easy to grab
    HoverHandler {
        id: hoverHandler
        cursorShape: root.vertical ? Qt.SplitHCursor : Qt.SplitVCursor
    }
}
