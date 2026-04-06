pragma Singleton
import QtQuick
import QtCore

// Persists SplitView divider positions and panel-slot assignments across
// application restarts using QSettings (via QtCore.Settings).
QtObject {
    id: root

    readonly property Settings _s: Settings {
        category: "layout"
    }

    // ── SplitView state ───────────────────────────────────────────────────────

    function saveSplitState(name, stateBytes) {
        _s.setValue(name, stateBytes)
    }

    function loadSplitState(name) {
        return _s.value(name, null)
    }

    // ── Panel assignment (panelId -> slotId) ──────────────────────────────────

    // Walk every slot in the registry, record which panel it holds, save as JSON.
    function savePanelAssignments(registry) {
        let assignment = {}
        for (let slotId in registry.slots) {
            let slot = registry.slots[slotId]
            if (slot && slot.hostedPanel) {
                for (let panelId in registry.panels) {
                    if (registry.panels[panelId] === slot.hostedPanel) {
                        assignment[panelId] = slotId
                        break
                    }
                }
            }
        }
        _s.setValue("panelAssignment", JSON.stringify(assignment))
    }

    // Load and apply saved assignment; falls back to the registry default.
    function restorePanelAssignments(registry) {
        let raw = _s.value("panelAssignment", "")
        if (!raw) return
        try {
            let assignment = JSON.parse(raw)
            registry.applyAssignment(assignment)
        } catch (e) {
            console.warn("LayoutStore: failed to parse saved panel assignment:", e)
        }
    }
}
