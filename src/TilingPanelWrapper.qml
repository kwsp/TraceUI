import QtQuick
import QtQuick.Controls
import TraceUI

// A slot in the tiling layout. Hosts exactly one panel at a time.
// Panels can be swapped by dragging their title bar onto another slot.
Item {
    id: root

    property string panelId: ""

    // The panel currently hosted in this slot. Setting this reparents the panel.
    property Item hostedPanel: null

    onHostedPanelChanged: {
        if (hostedPanel) {
            hostedPanel.parent = contentArea
            hostedPanel.anchors.fill = contentArea
            hostedPanel.visible = true
        }
    }

    // Called by another wrapper to take this slot's panel and give its own
    function swapWith(other) {
        if (!other || other === root) return
        let myPanel    = root.hostedPanel
        let theirPanel = other.hostedPanel
        root.hostedPanel  = null
        other.hostedPanel = null
        root.hostedPanel  = theirPanel
        other.hostedPanel = myPanel
        // Persist the new assignment
        if (typeof panelRegistry !== "undefined") {
            LayoutStore.savePanelAssignments(panelRegistry)
        }
    }

    // ── Drop area (whole slot accepts drops) ──────────────────────────────────
    DropArea {
        anchors.fill: parent
        keys: ["traceui/panel"]

        onEntered: (drag) => {
            if (drag.source !== root) {
                dropHighlight.visible = true
            }
        }
        onExited:  dropHighlight.visible = false
        onDropped: (drop) => {
            dropHighlight.visible = false
            let sourceWrapper = drop.source
            if (sourceWrapper && sourceWrapper !== root) {
                root.swapWith(sourceWrapper)
            }
        }
    }

    // Drop highlight overlay
    Rectangle {
        id: dropHighlight
        anchors.fill: parent
        color: Style.accentGold
        opacity: 0.12
        visible: false
        z: 5
    }

    // ── Border / background ───────────────────────────────────────────────────
    Rectangle {
        id: frame
        anchors.fill: parent
        color: "transparent"
        border.color: dragArea.active ? Style.accentGold : Style.borderDefault
        border.width: dragArea.active ? 2 : 1
        z: 1
    }

    // ── Title / drag bar ─────────────────────────────────────────────────────
    // The top 30 px of every panel is its header (each panel QML already renders
    // its own header text). We layer an invisible drag handle on top of it.
    Item {
        id: dragBar
        anchors.top:   parent.top
        anchors.left:  parent.left
        anchors.right: parent.right
        height: 30
        z: 10

        // Visual drag cursor
        HoverHandler {
            cursorShape: dragArea.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
        }

        DragHandler {
            id: dragArea
            target: null        // we control the Drag ourselves
            grabPermissions: PointerHandler.CanTakeOverFromHandlersOfDifferentType
        }

        // Drag metadata
        Drag.active:   dragArea.active
        Drag.source:   root           // the wrapper, not the bar
        Drag.keys:     ["traceui/panel"]
        Drag.hotSpot:  Qt.point(dragBar.width / 2, dragBar.height / 2)

        // Ghost image while dragging
        Drag.imageSource: ""   // let Qt capture the item
    }

    // ── Content area ─────────────────────────────────────────────────────────
    Item {
        id: contentArea
        anchors.fill: parent
        // hosted panel is reparented here
    }
}
