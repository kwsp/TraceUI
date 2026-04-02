import QtQuick
import TraceUI

Text {
    text: Qt.formatTime(new Date(), "hh:mm:ss")
    color: Style.foregroundColor
    font.family: "Hack"
    font.pixelSize: 32
    font.letterSpacing: 3

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: parent.text = Qt.formatTime(new Date(), "hh:mm:ss")
    }
}
