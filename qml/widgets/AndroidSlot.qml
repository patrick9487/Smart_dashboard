import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 200
    height: 120
    color: "#27AE60"
    border.color: "#229954"
    border.width: 2
    radius: 8

    Column {
        anchors.centerIn: parent
        spacing: 8

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Android Slot"
            font.pixelSize: 18
            font.bold: true
            color: "#ECF0F1"
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Ready"
            font.pixelSize: 24
            font.bold: true
            color: "#FFFFFF"
        }
    }
}


