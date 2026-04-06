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

    // Dot grid background
    ShaderEffect {
        anchors.fill: parent
        fragmentShader: "qrc:/shaders/dotgrid.frag.qsb"

        property vector2d resolution: Qt.vector2d(width, height)
        property real spacing: 20.0
        property real radius: 0.5
        property real dotAlpha: 0.2
    }

    // Main Tiling Layout
    SplitView {
        id: mainSplit
        anchors.fill: parent
        anchors.margins: 8
        orientation: Qt.Horizontal
        handle: SplitDivider {}

        // Left Column: System, Processes, Network
        SplitView {
            id: leftColumn
            SplitView.preferredWidth: 400
            SplitView.minimumWidth: 250
            orientation: Qt.Vertical
            handle: SplitDivider { vertical: false }

            TilingPanelWrapper {
                id: wrapperSys
                panelId: "system"
                SplitView.fillHeight: true
                SplitView.minimumHeight: 150
            }
            TilingPanelWrapper {
                id: wrapperProc
                panelId: "processes"
                SplitView.fillHeight: true
                SplitView.minimumHeight: 150
            }
            TilingPanelWrapper {
                id: wrapperNet
                panelId: "network"
                SplitView.fillHeight: true
                SplitView.minimumHeight: 150
            }
        }

        // Middle Column: Globe
        TilingPanelWrapper {
            id: wrapperGlobe
            panelId: "globe"
            SplitView.fillWidth: true
            SplitView.minimumWidth: 300
        }

        // Right Column: Terminal
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
            let savedLeft = LayoutStore.loadSplitState("leftColumn")
            if (savedLeft) leftColumn.restoreState(savedLeft)

            // Populate panels into their saved slots
            LayoutStore.restorePanelAssignments(panelRegistry)
        }
    }

    // All panel instances live here, parented to root so they survive reparenting
    SystemPanel  { id: sysPanel;  visible: false }
    ProcessPanel { id: procPanel; visible: false }
    NetworkPanel { id: netPanel;  visible: false }
    GlobePanel   { id: globePanel; visible: false }
    Loader {
        id: termLoader
        active: buildTerminal
        source: buildTerminal ? "qrc:/qt/qml/TraceUITerminal/TerminalPanel.qml" : ""
        visible: false
    }

    // Registry: panelId -> Item, slot id -> TilingPanelWrapper
    QtObject {
        id: panelRegistry

        property var panels: ({
            "system":    sysPanel,
            "processes": procPanel,
            "network":   netPanel,
            "globe":     globePanel,
            "terminal":  termLoader
        })

        property var slots: ({
            "system":    wrapperSys,
            "processes": wrapperProc,
            "network":   wrapperNet,
            "globe":     wrapperGlobe,
            "terminal":  wrapperTerm
        })

        // Default assignment: panelId -> slotId
        property var defaultAssignment: ({
            "system":    "system",
            "processes": "processes",
            "network":   "network",
            "globe":     "globe",
            "terminal":  "terminal"
        })

        Component.onCompleted: {
            applyAssignment(defaultAssignment)
        }

        function applyAssignment(assignment) {
            // Clear all slots first
            for (let slotId in slots) {
                slots[slotId].hostedPanel = null
            }
            // Place each panel in its assigned slot
            for (let panelId in assignment) {
                let slotId = assignment[panelId]
                let slot = slots[slotId]
                let panel = panels[panelId]
                if (slot && panel) {
                    slot.hostedPanel = panel
                }
            }
        }
    }

    // Save layout on close
    onClosing: {
        LayoutStore.saveSplitState("mainSplit",  mainSplit.saveState())
        LayoutStore.saveSplitState("leftColumn", leftColumn.saveState())
        LayoutStore.savePanelAssignments(panelRegistry)
    }
}
