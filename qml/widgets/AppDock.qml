import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: dock
    width: parent ? parent.width : 800
    height: 140

    readonly property bool waydroidAvailable: (typeof Waydroid !== "undefined") && Waydroid !== null
    readonly property bool waydroidRunning: waydroidAvailable && Waydroid.running

    // 整體背景（稍微透明，讓上方畫面隱約透出）
    Rectangle {
        anchors.fill: parent
        color: "#00000000"
    }

    // 底部 Dock 底欄（類似早期 macOS Leopard 的半透明底座）
    Rectangle {
        id: shelf
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 70
        radius: 24
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#252932" }
            GradientStop { position: 0.5; color: "#181b23" }
            GradientStop { position: 1.0; color: "#111219" }
        }
        border.color: "#3a3f4a"
        border.width: 1
        opacity: 0.94
    }

    // 上緣高光線，讓底欄更有立體感
    Rectangle {
        anchors.left: shelf.left
        anchors.right: shelf.right
        anchors.top: shelf.top
        height: 1
        color: "#ffffff33"
    }

    // 狀態列：顯示 Waydroid 狀態 / App 數量 + 控制按鈕
    Row {
        id: statusRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: shelf.top
        anchors.margins: 16
        spacing: 12

        Text {
            id: statusText
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - (controlButton.visible ? controlButton.implicitWidth + 12 : 0)
            text: !waydroidAvailable ? qsTr("Waydroid unavailable")
                 : waydroidRunning
                     ? (Waydroid.appsModel.count > 0
                        ? qsTr("Android Apps: %1").arg(Waydroid.appsModel.count)
                        : qsTr("Loading apps from Waydroid…"))
                     : qsTr("Starting Waydroid…")
            elide: Text.ElideRight
            color: !waydroidAvailable ? "#f07f7f"
                   : waydroidRunning ? "#8fe6c2" : "#f7b35a"
            font.pixelSize: 14
        }

        Button {
            id: controlButton
            anchors.verticalCenter: parent.verticalCenter
            visible: waydroidAvailable
            text: waydroidRunning ? qsTr("Reload") : qsTr("Start")
            onClicked: {
                if (!waydroidAvailable)
                    return;
                if (waydroidRunning)
                    Waydroid.refreshApps();
                else
                    Waydroid.startSession();
            }
        }
    }

    // App 圖示列：貼在底欄上方，支援拖曳與滑鼠滾輪水平捲動
    Flickable {
        id: flick
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: shelf.bottom
        anchors.bottomMargin: 8
        height: 80

        contentWidth: row.width
        contentHeight: row.height
        clip: true
        interactive: contentWidth > width

        Row {
            id: row
            spacing: 24
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter

            Repeater {
                model: waydroidAvailable ? Waydroid.appsModel : 0
                delegate: AppIcon {
                    appTitle: model.label
                    packageId: model.package
                    enabled: waydroidRunning
                }
            }
        }

        // 使用滑鼠滾輪進行水平捲動
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton   // 不攔截滑鼠按鍵，只處理滾輪
            hoverEnabled: true

            onWheel: {
                var delta = wheel.angleDelta.y !== 0 ? wheel.angleDelta.y : wheel.angleDelta.x;
                // 調整速度係數讓手感自然一些
                flick.contentX -= delta * 0.4;
                wheel.accepted = true;
            }
        }
    }
}
