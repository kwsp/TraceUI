pragma Singleton
import QtQuick

QtObject {
    // Neo-Kitsch palette based on Architecture.md
    readonly property color background:       "#0d0b0e"
    readonly property color panelBg:          Qt.rgba(0.10, 0.08, 0.12, 0.88)
    readonly property color borderDefault:    "#8a7a50"
    readonly property color borderAlert:      "#c9a84c"
    readonly property color borderError:      "#8a3030"
    readonly property color textPrimary:      "#e8dfc0"
    readonly property color textDim:          Qt.rgba(0.91, 0.87, 0.75, 0.40)
    readonly property color textLabel:        "#7a6e50"
    readonly property color accentGold:       "#c9a84c"
    readonly property color accentSilver:     "#a8b0b8"
    readonly property color accentError:      "#8a3030"
    readonly property color foregroundColor:  "#e8dfc0"
    readonly property color backgroundColor:  "#0d0b0e"

    readonly property string fontDisplay: "Blender Pro Book"
    readonly property string fontData:    "Hack"
}
