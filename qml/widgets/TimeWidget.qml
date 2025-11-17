import QtQuick

Item {
    id: root
    implicitWidth: 200
    implicitHeight: 60

    Text {
        id: timeText
        anchors.left: parent.left
        anchors.leftMargin: root.width * 0.02    // 2% 左邊距
        anchors.verticalCenter: parent.verticalCenter
        text: "00 : 00"
        color: "white"
        font.bold: true
        font.pixelSize: root.height * 0.7        // 字體高度佔元件高度的 70%
    }
}
