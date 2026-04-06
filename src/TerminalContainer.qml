import QtQuick
import TraceUI

// Container for terminal panel with turn-on reveal animation.
// Manages the reveal sequence: lines appear, expand, then terminal content fades in.
Item {
    id: root
    implicitWidth: 500
    implicitHeight: 400
    clip: true

    // ── Public API ───────────────────────────────────────────────────────────
    property alias reveal: turnOnReveal
    property alias terminal: terminalLoader.item

    signal revealComplete()

    function startReveal() {
        turnOnReveal.start()
    }

    // ── Turn-on reveal animation ──────────────────────────────────────────────
    TurnOnReveal {
        id: turnOnReveal
        anchors.fill: parent
        z: 100

        onExpandComplete: {
            // Terminal content fades in between the lines
            terminalFadeIn.start()
        }

        onDone: {
            root.revealComplete()
        }
    }

    // ── Terminal content ──────────────────────────────────────────────────────
    Loader {
        id: terminalLoader
        anchors.fill: parent
        active: buildTerminal
        source: "qrc:/qt/qml/TraceUITerminal/TerminalPanel.qml"
        opacity: 0
        z: 50
    }

    // ── Content fade-in animation ─────────────────────────────────────────────
    NumberAnimation {
        id: terminalFadeIn
        target: terminalLoader
        property: "opacity"
        from: 0; to: 1
        duration: AnimConfig.termExpandDur
        easing.type: AnimConfig.termExpandEasing
    }
}
