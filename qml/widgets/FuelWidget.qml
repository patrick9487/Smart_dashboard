import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 200
    height: 120
    color: "#34495E"
    border.color: "#7F8C8D"
    border.width: 2
    radius: 8

    Column {
        anchors.centerIn: parent
        spacing: 8

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Fuel"
            font.pixelSize: 18
            font.bold: true
            color: "#ECF0F1"
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "85%"
            font.pixelSize: 32
            font.bold: true
            color: "#E67E22"
        }
    }
}


