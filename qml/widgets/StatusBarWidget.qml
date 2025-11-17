import QtQuick
import QtQuick.Layouts

Item {
    id: root
    implicitWidth: 600
    implicitHeight: 60

    Rectangle {
        anchors.fill: parent
        color: "#181820"
        border.color: "#2a2a34"
        border.width: root.height * 0.03      // 邊框厚度比例化
        radius: root.height * 0.1             // 微圓角，隨比例變
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: root.height * 0.25   // 上下左右邊距比例化
        spacing: root.width * 0.04            // 左右間距比例化

        Text {
            text: "VVA"
            color: "white"
            font.bold: true
            font.pixelSize: root.height * 0.37   // 字體比例化
        }

        Item {
            Layout.fillWidth: true
        }

        Text {
            text: "TCS ON"
            color: "white"
            font.bold: true
            font.pixelSize: root.height * 0.37
        }
    }
}
