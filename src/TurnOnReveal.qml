import QtQuick
import TraceUI

// Reusable "turn on" reveal animation: two horizontal lines appear at center,
// then expand outward (up/down). Lines hold position as borders, then signal
// expandComplete so the caller can fade in content.
//
// Usage:
//   TurnOnReveal {
//       anchors.fill: parent
//       onExpandComplete: contentFadeAnim.start()
//       onDone: ...
//   }

Item {
    id: root

    // ── Configuration ────────────────────────────────────────────────────────
    property color lineColor:       Style.accentGold
    property int   lineThickness:   1
    property int   appearDuration:  AnimConfig.termLineAppearDur
    property int   expandDuration:  AnimConfig.termExpandDur
    property int   contentDelay:    AnimConfig.termContentDelay
    property int   contentFadeDur:  AnimConfig.termContentFadeDur
    property int   easingType:      AnimConfig.termExpandEasing

    // ── Read-only state ──────────────────────────────────────────────────────
    readonly property bool running: _running
    property real _halfHeight: 0
    property bool _running: false

    // ── Signals ──────────────────────────────────────────────────────────────
    signal expandComplete()   // Lines at edges; content can start fading in
    signal done()             // Entire sequence finished (incl. content fade time)

    // ── API ──────────────────────────────────────────────────────────────────
    function start() {
        if (_running) return
        _running = true
        _lineAppearAnim.start()
    }

    function reset() {
        _lineAppearAnim.stop()
        _lineExpandAnim.stop()
        _contentDelayTimer.stop()
        _doneTimer.stop()
        _running = false
        _halfHeight = 0
        lineTop.opacity = 0
        lineBottom.opacity = 0
    }

    // ── Lines ────────────────────────────────────────────────────────────────
    Rectangle {
        id: lineTop
        anchors.left: parent.left
        anchors.right: parent.right
        y: parent.height / 2 - root._halfHeight
        height: root.lineThickness
        color: root.lineColor
        opacity: 0
        visible: root._running
    }

    Rectangle {
        id: lineBottom
        anchors.left: parent.left
        anchors.right: parent.right
        y: parent.height / 2 + root._halfHeight - root.lineThickness
        height: root.lineThickness
        color: root.lineColor
        opacity: 0
        visible: root._running
    }

    // ── Internal animations ──────────────────────────────────────────────────

    // Step 1: Fade lines in at center
    SequentialAnimation {
        id: _lineAppearAnim
        ParallelAnimation {
            NumberAnimation {
                target: lineTop; property: "opacity"
                from: 0; to: 1; duration: root.appearDuration
            }
            NumberAnimation {
                target: lineBottom; property: "opacity"
                from: 0; to: 1; duration: root.appearDuration
            }
        }
        ScriptAction { script: _lineExpandAnim.start() }
    }

    // Step 2: Expand lines outward
    NumberAnimation {
        id: _lineExpandAnim
        target: root; property: "_halfHeight"
        from: 1; to: root.height / 2
        duration: root.expandDuration
        easing.type: root.easingType
        onFinished: _contentDelayTimer.start()
    }

    // Step 3: Pause, then signal expandComplete
    Timer {
        id: _contentDelayTimer
        interval: root.contentDelay
        onTriggered: {
            root.expandComplete()
            _doneTimer.start()
        }
    }

    // Step 4: Wait for content fade to finish, then signal done and clean up
    Timer {
        id: _doneTimer
        interval: root.contentFadeDur
        onTriggered: {
            lineTop.visible = false
            lineBottom.visible = false
            root.done()
        }
    }
}
