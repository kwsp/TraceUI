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

    // ── Phase tracking ───────────────────────────────────────────────────────
    // Phases: 0 = blank, 1 = terminal reveal, 2 = panels fade in, 3 = done
    // Globe intro is independent — controlled by Globe.startupDelay
    property int animPhase: 0

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
            SplitView.preferredWidth: 500
            SplitView.minimumWidth: 300
            opacity: 0
        }

        // Right Column (Network) - fills remaining space
        TilingPanelWrapper {
            id: wrapperRight
            panelId: "network"
            SplitView.fillWidth: true
            SplitView.minimumWidth: 300
            opacity: 0
        }

        // Terminal Column (Optional) - wrapped with reveal animation
        TilingPanelWrapper {
            id: wrapperTerm
            panelId: "terminal"
            visible: buildTerminal
            SplitView.preferredWidth: 500
            SplitView.minimumWidth: 300

            // Terminal container with built-in reveal animation
            TerminalContainer {
                id: terminalContainer
                anchors.fill: parent
                terminalActive: buildTerminal

                onRevealComplete: root.animPhase = 2
            }
        }

        // Restore layout state on startup
        Component.onCompleted: {
            let savedMain = LayoutStore.loadSplitState("mainSplit")
            if (savedMain) mainSplit.restoreState(savedMain)
            // Populate panels into their saved slots
            LayoutStore.restorePanelAssignments(panelRegistry)
        }
    }

    // ── Panel fade-in animations ─────────────────────────────────────────────
    ParallelAnimation {
        id: panelFadeAnim

        NumberAnimation {
            target: wrapperLeft
            property: "opacity"
            from: 0; to: 1
            duration: AnimConfig.panelFadeDur
            easing.type: AnimConfig.panelFadeEasing
        }
        NumberAnimation {
            target: wrapperRight
            property: "opacity"
            from: 0; to: 1
            duration: AnimConfig.panelFadeDur
            easing.type: AnimConfig.panelFadeEasing
        }

        onFinished: root.animPhase = 3  // done
    }

    Timer {
        id: panelFadeTimer
        interval: AnimConfig.panelFadeDelay
        onTriggered: panelFadeAnim.start()
    }

    // ── Phase state machine ──────────────────────────────────────────────────
    // Globe intro is handled by Globe.startupDelay (AnimConfig.globeAbsoluteStart)
    onAnimPhaseChanged: {
        switch (animPhase) {
        case 1:
            terminalContainer.startReveal()
            break
        case 2:
            panelFadeTimer.start()
            break
        case 3:
            // Animation complete — globe starts on its own via startupDelay
            break
        }
    }

    // ── Startup kick-off ─────────────────────────────────────────────────────
    Timer {
        id: startupTimer
        interval: AnimConfig.startDelay
        running: true
        onTriggered: {
            if (buildTerminal) {
                root.animPhase = 1
            } else {
                // No terminal: skip straight to panel fade
                root.animPhase = 2
            }
        }
    }

    // All panel instances live here, parented to root so they survive reparenting
    SystemPanel  { id: sysPanel;  visible: false }
    NetworkPanel { id: netPanel;  visible: false }

    // Registry: panelId -> Item, slot id -> TilingPanelWrapper
    QtObject {
        id: panelRegistry
        property var panels: ({ "system": sysPanel, "network": netPanel, "terminal": terminalContainer })
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
