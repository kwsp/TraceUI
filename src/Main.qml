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
    // Phases: 0 = blank, 1 = terminal line appears, 2 = lines expand,
    //         3 = terminal content fades in, 4 = panels fade in,
    //         5 = globe intro, 6 = done
    property int animPhase: 0

    // ── Terminal reveal animation state ──────────────────────────────────────
    property real termRevealHalfHeight: 0

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

        // Terminal Column (Optional)
        TilingPanelWrapper {
            id: wrapperTerm
            panelId: "terminal"
            visible: buildTerminal
            SplitView.preferredWidth: 500
            SplitView.minimumWidth: 300
            clip: true
            opacity: 0
        }

        // Restore layout state on startup
        Component.onCompleted: {
            let savedMain = LayoutStore.loadSplitState("mainSplit")
            if (savedMain) mainSplit.restoreState(savedMain)
            // Populate panels into their saved slots
            LayoutStore.restorePanelAssignments(panelRegistry)
        }
    }

    // ── Terminal reveal lines (outside wrapper so visible during opacity=0) ───
    // Position computed from wrapperTerm's screen position

    // Top reveal line (moves upward from center)
    Rectangle {
        id: termLineTop
        visible: buildTerminal && root.animPhase >= 1 && root.animPhase <= 3
        z: 500
        width: wrapperTerm.width
        height: 2
        color: Style.accentGold
        opacity: 0

        // Position in root coordinates
        x: wrapperTerm.mapToItem(root.contentItem, 0, 0).x
        y: {
            let termCenterY = wrapperTerm.mapToItem(root.contentItem, 0, 0).y + wrapperTerm.height / 2
            return termCenterY - root.termRevealHalfHeight
        }
    }

    // Bottom reveal line (moves downward from center)
    Rectangle {
        id: termLineBottom
        visible: buildTerminal && root.animPhase >= 1 && root.animPhase <= 3
        z: 500
        width: wrapperTerm.width
        height: 2
        color: Style.accentGold
        opacity: 0

        // Position in root coordinates
        x: wrapperTerm.mapToItem(root.contentItem, 0, 0).x
        y: {
            let termCenterY = wrapperTerm.mapToItem(root.contentItem, 0, 0).y + wrapperTerm.height / 2
            return termCenterY + root.termRevealHalfHeight - 2
        }
    }

    // ── Terminal reveal animations ───────────────────────────────────────────

    // Phase 1: Center line appears (both lines at same position)
    SequentialAnimation {
        id: termLineAnim
        ParallelAnimation {
            NumberAnimation {
                target: termLineTop
                property: "opacity"
                from: 0; to: 1
                duration: AnimConfig.termLineAppearDur
            }
            NumberAnimation {
                target: termLineBottom
                property: "opacity"
                from: 0; to: 1
                duration: AnimConfig.termLineAppearDur
            }
        }
        ScriptAction { script: root.animPhase = 2 }
    }

    // Phase 2: Lines expand outward (up/down) - no content yet
    NumberAnimation {
        id: termExpandAnim
        target: root
        property: "termRevealHalfHeight"
        from: 1; to: wrapperTerm.height / 2
        duration: AnimConfig.termExpandDur
        easing.type: AnimConfig.termExpandEasing
        onFinished: termContentDelayTimer.start()
    }

    // Delay before terminal content appears
    Timer {
        id: termContentDelayTimer
        interval: AnimConfig.termContentDelay
        onTriggered: root.animPhase = 3
    }

    // Phase 3: Terminal content fades in (lines become borders)
    NumberAnimation {
        id: termContentAnim
        target: wrapperTerm
        property: "opacity"
        from: 0; to: 1
        duration: AnimConfig.termExpandDur
        easing.type: AnimConfig.termExpandEasing
        onFinished: {
            termLineTop.visible = false
            termLineBottom.visible = false
            root.animPhase = 4
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

        onFinished: root.animPhase = 5
    }

    // ── Globe intro trigger ──────────────────────────────────────────────────
    property bool globeReady: false

    Timer {
        id: panelFadeTimer
        interval: AnimConfig.panelFadeDelay
        onTriggered: panelFadeAnim.start()
    }

    Timer {
        id: globeStartTimer
        interval: AnimConfig.globeStartDelay
        onTriggered: {
            root.globeReady = true
            root.animPhase = 6
        }
    }

    // ── Phase state machine ──────────────────────────────────────────────────
    onAnimPhaseChanged: {
        switch (animPhase) {
        case 1:
            termLineAnim.start()
            break
        case 2:
            termExpandAnim.start()
            break
        case 3:
            termContentAnim.start()
            break
        case 4:
            panelFadeTimer.start()
            break
        case 5:
            globeStartTimer.start()
            break
        case 6:
            // Animation complete
            break
        }
    }

    // ── Startup kick-off ─────────────────────────────────────────────────────
    // Delay slightly to ensure SplitView layout is complete
    Timer {
        id: startupTimer
        interval: AnimConfig.startDelay
        running: true
        onTriggered: {
            if (buildTerminal) {
                root.animPhase = 1
            } else {
                // No terminal: skip straight to panel fade
                root.animPhase = 4
            }
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
