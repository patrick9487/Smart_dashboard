import QtQuick

Item {
    id: root
    width: 240
    height: 70

    Rectangle {
        anchors.fill: parent
        radius: 12
        color: "#2d2222"
        border.color: "#c34545"
        border.width: 2
    }

    Text {
        anchors.centerIn: parent
        text: "Warning"
        color: "white"
        font.pixelSize: 24
        font.bold: true
    }
}
