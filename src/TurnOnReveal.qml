import QtQuick
import TraceUI

// Reusable "turn on" reveal animation: two horizontal lines appear at center,
// then expand outward (up/down) to reveal content. Lines become top/bottom borders.
//
// Usage:
//   TurnOnReveal {
//       id: reveal
//       anchors.fill: parent
//       onExpandComplete: myContent.opacity = 1
//       onDone: reveal.visible = false
//   }
//   Component.onCompleted: reveal.start()

Item {
    id: root
    clip: true

    // ── Configuration ────────────────────────────────────────────────────────
    property color lineColor: Style.accentGold
    property int lineThickness: 2
    property int appearDuration: AnimConfig.termLineAppearDur   // fade-in time
    property int expandDuration: AnimConfig.termExpandDur       // expansion time
    property int contentDelay: AnimConfig.termContentDelay      // pause before expandComplete
    property int easingType: AnimConfig.termExpandEasing

    // ── State ─────────────────────────────────────────────────────────────────
    property real halfHeight: 0
    property bool running: false

    // ── Signals ───────────────────────────────────────────────────────────────
    signal expandComplete()   // Lines finished expanding, content can fade in
    signal done()             // Full animation complete, hide lines

    // ── API ───────────────────────────────────────────────────────────────────
    function start() {
        if (running) return
        running = true
        lineAppearAnim.start()
    }

    function reset() {
        running = false
        halfHeight = 0
        lineTop.opacity = 0
        lineBottom.opacity = 0
        lineTop.visible = true
        lineBottom.visible = true
    }

    // ── Lines ─────────────────────────────────────────────────────────────────
    Rectangle {
        id: lineTop
        anchors.left: parent.left
        anchors.right: parent.right
        y: parent.height / 2 - root.halfHeight
        height: root.lineThickness
        color: root.lineColor
        opacity: 0
        visible: root.running
    }

    Rectangle {
        id: lineBottom
        anchors.left: parent.left
        anchors.right: parent.right
        y: parent.height / 2 + root.halfHeight - root.lineThickness
        height: root.lineThickness
        color: root.lineColor
        opacity: 0
        visible: root.running
    }

    // ── Animations ────────────────────────────────────────────────────────────

    // Phase 1: Lines appear at center (opacity only)
    SequentialAnimation {
        id: lineAppearAnim
        ParallelAnimation {
            NumberAnimation {
                target: lineTop
                property: "opacity"
                from: 0; to: 1
                duration: root.appearDuration
            }
            NumberAnimation {
                target: lineBottom
                property: "opacity"
                from: 0; to: 1
                duration: root.appearDuration
            }
        }
        ScriptAction { script: lineExpandAnim.start() }
    }

    // Phase 2: Lines expand outward
    NumberAnimation {
        id: lineExpandAnim
        target: root
        property: "halfHeight"
        from: 1; to: root.height / 2
        duration: root.expandDuration
        easing.type: root.easingType
        onFinished: contentDelayTimer.start()
    }

    // Phase 3: Pause before signaling expandComplete
    Timer {
        id: contentDelayTimer
        interval: root.contentDelay
        onTriggered: {
            root.expandComplete()
            doneTimer.start()
        }
    }

    // Phase 4: Signal done after content has time to appear
    Timer {
        id: doneTimer
        interval: root.expandDuration
        onTriggered: {
            lineTop.visible = false
            lineBottom.visible = false
            root.done()
        }
    }
}
