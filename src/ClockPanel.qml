import QtQuick

Text {
    text: Qt.formatTime(new Date(), "hh:mm:ss")
    color: "#e8dfc0"
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
