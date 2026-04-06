import QtQuick
import QtQuick.Controls
import TraceUI

// Custom handle for SplitView — a thin line that appears on hover.
Rectangle {
    id: root

    // True for horizontal SplitView (handle is vertical bar), false for vertical.
    property bool vertical: true

    implicitWidth:  vertical ? 6 : 12
    implicitHeight: vertical ? 12 : 6

    color: "transparent"

    // The visible line - invisible by default, shows on hover
    Rectangle {
        anchors.centerIn: parent
        width:  root.vertical ? 1 : parent.width
        height: root.vertical ? parent.height : 1
        color: hoverHandler.hovered || SplitHandle.pressed
               ? Style.accentGold
               : "transparent"

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }

    // Hover handler for cursor
    HoverHandler {
        id: hoverHandler
        cursorShape: root.vertical ? Qt.SplitHCursor : Qt.SplitVCursor
    }
}
