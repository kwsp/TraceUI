pragma Singleton
import QtQuick

QtObject {
    // ── Startup Animation Timing ─────────────────────────────────────────────
    // All durations in milliseconds.

    // Phase 0: Initial pause on empty screen
    readonly property int startDelay:           500

    // Phase 1: Terminal reveal — horizontal line appears, then expands upward
    readonly property int termLineAppearDur:    300   // line fades/scales in
    readonly property int termExpandDur:        600   // line expands to full height
    readonly property int termExpandEasing:     Easing.OutCubic

    // Phase 2: System & Network panels fade in (starts after terminal reveal)
    readonly property int panelFadeDelay:       200   // delay after terminal finishes
    readonly property int panelFadeDur:         800   // fade-in duration
    readonly property int panelFadeEasing:      Easing.OutQuad

    // Phase 3: Globe intro begins (starts after panels are visible)
    readonly property int globeStartDelay:      300   // delay after panels finish
    readonly property int globeIntroDur:        2000  // globe's own intro animation

    // ── Computed start offsets (absolute from t=0) ───────────────────────────
    readonly property int termLineStart:    startDelay
    readonly property int termExpandStart:  termLineStart + termLineAppearDur
    readonly property int panelFadeStart:   termExpandStart + termExpandDur + panelFadeDelay
    readonly property int globeStart:       panelFadeStart + panelFadeDur + globeStartDelay
}
