import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 96
    height: 96

    property string appTitle: "App"
    property string packageId: ""
    property url iconSource: ""
    property bool enabled: true
    
    signal appClicked(string packageId)

    Rectangle {
        id: tile
        anchors.fill: parent
        radius: 20
        color: enabled ? "#1f222a" : "#2a2d33"
        border.color: "#353a45"
        border.width: 1.5
        opacity: enabled ? 1.0 : 0.6

        Column {
            anchors.centerIn: parent
            spacing: 6
            Image {
                source: root.iconSource
                width: 48
                height: 48
                fillMode: Image.PreserveAspectFit
                visible: source !== ""
                opacity: enabled ? 1.0 : 0.6
            }

            Text {
                width: root.width * 0.85
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                text: root.appTitle
                font.pixelSize: 14
                color: "white"
                opacity: enabled ? 1.0 : 0.6
                maximumLineCount: 2
                elide: Text.ElideRight
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        enabled: root.enabled
        onClicked: {
            if (root.enabled && root.packageId.length > 0) {
                // 發出信號，讓父組件處理（用於嵌入模式）
                root.appClicked(root.packageId)
                // 同時也啟動應用（如果需要在獨立視窗中運行）
                if (typeof Waydroid !== "undefined" && Waydroid !== null) {
                    Waydroid.launchApp(root.packageId);
                }
            }
        }
        onPressedChanged: tile.color = pressed ? "#2a2f3c" : (enabled ? "#1f222a" : "#2a2d33")
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
}
