import QtQuick
import QtQuick.Controls
import TraceUI

Window {
    id: root

    // buildTerminal is now a context property

    flags: Qt.FramelessWindowHint
    visibility: Window.FullScreen
    color: Style.backgroundColor
    visible: true

    FontLoader {
        source: "qrc:/fonts/BlenderPro-Book.ttf"
    }
    FontLoader {
        source: "qrc:/fonts/Hack-Regular.ttf"
    }

    // 1. Dot grid background
    ShaderEffect {
        anchors.fill: parent
        fragmentShader: "qrc:/shaders/dotgrid.frag.qsb"
        property vector2d resolution: Qt.vector2d(width, height)
        property real spacing: 20.0
        property real radius: 0.5
        property real dotAlpha: 0.2
    }

    // 2. Global Scanline Overlay
    ShaderEffect {
        anchors.fill: parent
        z: 1000 // On top of everything
        opacity: 0.05
        fragmentShader: "qrc:/shaders/scanlines.frag.qsb"
    }

    // 3. Main Tiling Layout
    SplitView {
        id: mainSplit
        anchors.fill: parent
        anchors.margins: 8
        orientation: Qt.Horizontal
        handle: SplitDivider {}

        // Left Column (System)
        TilingPanelWrapper {
            id: wrapperLeft
            panelId: "system"
            SplitView.fillWidth: true
            SplitView.minimumWidth: 400
        }

        // Right Column (Network)
        TilingPanelWrapper {
            id: wrapperRight
            panelId: "network"
            SplitView.fillWidth: true
            SplitView.minimumWidth: 400
        }

        // Terminal Column (Optional)
        TilingPanelWrapper {
            id: wrapperTerm
            panelId: "terminal"
            visible: buildTerminal
            SplitView.preferredWidth: 600
            SplitView.minimumWidth: 300
        }

        // Restore layout state on startup
        Component.onCompleted: {
            let savedMain = LayoutStore.loadSplitState("mainSplit")
            if (savedMain) mainSplit.restoreState(savedMain)
            // Populate panels into their saved slots
            LayoutStore.restorePanelAssignments(panelRegistry)
        }
    }

    // All panel instances live here, parented to root so they survive reparenting
    SystemPanel  { id: sysPanel;  visible: false }
    NetworkPanel { id: netPanel;  visible: false }
    Loader {
        id: termLoader
        active: buildTerminal
        source: buildTerminal ? "qrc:/qt/qml/TraceUITerminal/TerminalPanel.qml" : ""
        visible: false
    }

    // Registry: panelId -> Item, slot id -> TilingPanelWrapper
    QtObject {
        id: panelRegistry
        property var panels: ({ "system": sysPanel, "network": netPanel, "terminal": termLoader })
        property var slots:  ({ "left": wrapperLeft, "right": wrapperRight, "terminal": wrapperTerm })
        property var defaultAssignment: ({ "system": "left", "network": "right", "terminal": "terminal" })

        Component.onCompleted: applyAssignment(defaultAssignment)

        function applyAssignment(assignment) {
            for (let slotId in slots) slots[slotId].hostedPanel = null
            for (let panelId in assignment) {
                let slotId = assignment[panelId]; let slot = slots[slotId]; let panel = panels[panelId]
                if (slot && panel) slot.hostedPanel = panel
            }
        }
    }

    // Save layout on close
    onClosing: {
        LayoutStore.saveSplitState("mainSplit",  mainSplit.saveState())
        LayoutStore.savePanelAssignments(panelRegistry)
    }
}
