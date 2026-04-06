import QtQuick
import TraceUI

// Container for terminal panel with turn-on reveal animation.
// Manages the reveal sequence: lines appear → expand → pause → content fades in.
Item {
    id: root
    implicitWidth: 500
    implicitHeight: 400

    // ── Configuration ────────────────────────────────────────────────────────
    property alias reveal: turnOnReveal
    property alias terminal: terminalLoader.item
    property bool  terminalActive: true
    property url   terminalSource: "qrc:/qt/qml/TraceUITerminal/TerminalPanel.qml"

    // ── Signals ──────────────────────────────────────────────────────────────
    signal revealComplete()

    // ── API ──────────────────────────────────────────────────────────────────
    function startReveal() {
        turnOnReveal.start()
    }

    // ── Turn-on reveal animation ──────────────────────────────────────────────
    TurnOnReveal {
        id: turnOnReveal
        anchors.fill: parent
        z: 100

        onExpandComplete: terminalFadeIn.start()
        onDone: root.revealComplete()
    }

    // ── Terminal content ──────────────────────────────────────────────────────
    Loader {
        id: terminalLoader
        anchors.fill: parent
        active: root.terminalActive
        source: root.terminalSource
        opacity: 0
    }

    // ── Content fade-in ──────────────────────────────────────────────────────
    NumberAnimation {
        id: terminalFadeIn
        target: terminalLoader
        property: "opacity"
        from: 0; to: 1
        duration: turnOnReveal.contentFadeDur
        easing.type: turnOnReveal.easingType
    }
}
