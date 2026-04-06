pragma Singleton
import QtQuick

QtObject {
    // ── Startup Animation Timing ─────────────────────────────────────────────
    // All durations in milliseconds.

    // Phase 0: Initial pause on empty screen
    readonly property int startDelay:           500

    // Phase 1: Terminal reveal — horizontal lines appear, then expand outward
    readonly property int termLineAppearDur:    300   // line fade-in time
    readonly property int termExpandDur:        600   // lines expand to full height
    readonly property int termExpandEasing:     Easing.OutCubic
    readonly property int termContentDelay:     300   // pause after lines reach edges
    readonly property int termContentFadeDur:   500   // terminal content fade-in

    // Phase 2: System & Network panels fade in
    readonly property int panelFadeDelay:       200   // delay after terminal reveal completes
    readonly property int panelFadeDur:         800   // fade-in duration
    readonly property int panelFadeEasing:      Easing.OutQuad

    // Phase 3: Globe intro begins
    readonly property int globeStartDelay:      300   // delay after panels are visible
    readonly property int globeIntroDur:        2000  // globe's own intro animation
}
