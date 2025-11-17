import QtQuick

Item {
    id: root
    // 外部沒給就用這個
    implicitWidth: 360
    implicitHeight: 120

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: root.height * 0.15   // 原本 20 -> 比例化
        spacing: root.height * 0.1              // 間距也比例化

        Text {
            text: "ODO"
            color: "#cfd5dd"
            font.pixelSize: root.height * 0.22   // 原本大約 26/120 ≈ 0.21
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: "888888 km"
            color: "white"
            font.pixelSize: root.height * 0.28   // 原本 34/120 ≈ 0.28
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
