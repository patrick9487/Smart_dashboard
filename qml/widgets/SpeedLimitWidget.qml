import QtQuick

Item {
    id: root
    implicitWidth: 90
    implicitHeight: 90

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: "#ffffff"
        border.color: "#ff3a3a"
        border.width: root.width * 0.09   // 依照寬度比例化
    }

    Text {
        anchors.centerIn: parent
        text: "60"
        font.pixelSize: root.height * 0.38   // 字體依照高度比例化
        font.bold: true
        color: "#1c1f26"
    }
}
