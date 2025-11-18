import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "widgets"

ApplicationWindow {
    id: window
    visible: true
    width: 1170
    height: 665
    title: "Smart Dashboard"
    color: "#101015"

    // Waydroid 是否可用（避免 Waydroid 為 null）
    property bool waydroidAvailable: typeof Waydroid !== "undefined"
                                     && Waydroid !== null
                                     && Waydroid.appsModel
    
    // Waydroid 是否有 app（用於隱藏 ODO/StatusBar）
    property bool appsAvailable: waydroidAvailable
                                 && Waydroid.appsModel.count > 0

    // 背景漸層
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#16181f" }
            GradientStop { position: 0.5; color: "#0f1017" }
            GradientStop { position: 1.0; color: "#070810" }
        }
    }

    // 左上時間
    TimeWidget {
        id: timeWidget
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 5
    }

    // 底部狀態列（Waydroid 有 app 時隱藏）
    StatusBarWidget {
        id: statusBar
        visible: !appsAvailable
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    // ODO：貼在狀態列上（Waydroid 有 app 時隱藏）
    OdometerWidget {
        id: odometer
        visible: !appsAvailable
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: statusBar.bottom
        anchors.bottomMargin: 0
        width: parent.width * 0.35
        height: parent.height * 0.17
    }

    // ================== 中間速度表 ==================
    SpeedWidget {
        id: speed
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: timeWidget.bottom
        anchors.topMargin: window.height * 0.005
        anchors.bottom: appsAvailable ? appDock.top : odometer.top
        anchors.bottomMargin: appsAvailable ? window.height * 0.05 : window.height * 0.05

        // 原本 0.5 太寬，改 0.8 留一點給左右條
        width: parent.width * 0.8
    }

    // ================== 左邊轉速，跟速度表同高 ==================
    TachometerWidget {
        id: tachometer
        anchors.verticalCenter: speed.verticalCenter
        anchors.verticalCenterOffset: speed.height * 0.026
        height: speed.height * 1.05
        width: Math.min(height * 0.38, window.width * 0.14)

        anchors.right: speed.left

        // 可調參數
        property real baseGapPx: 20                 // 固定基準距離
        property real scaleFactor: 0.005            // 跟 speed 寬度成比例
        property real scaleCap: 15                  // 最多增加的像素上限
        property real scaleGapPx: Math.min(scaleCap, speed.width * scaleFactor)

        property real overlapRatio: 0.70            // 插入比例
        property real safetyPx: 8
        property real maxInsertRatio: 0.7

        property real desiredRM: baseGapPx + scaleGapPx
                                 - (tachometer.width * overlapRatio)
                                 - safetyPx

        anchors.rightMargin: Math.max(
                                -tachometer.width * maxInsertRatio,
                                desiredRM
                             )
    }

    // ================== 右邊油量，跟速度表同高 ==================
    FuelGaugeWidget {
        id: fuel
        anchors.verticalCenter: speed.verticalCenter
        height: speed.height
        width: Math.min(height * 0.38, window.width * 0.14)
        anchors.left: speed.right

        // 根據 speed 寬度決定距離，越寬越遠
        anchors.leftMargin: Math.min(-180, -190 + speed.width * 0.000001)
    }

    // ================== 底部 App Dock（Waydroid 有 app 時顯示） ==================
    AppDock {
        id: appDock
        visible: appsAvailable  // 只有在有 app 時才顯示
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 0
        anchors.rightMargin: 0
        anchors.bottomMargin: 16   // 與視窗底部保留一點空間
        height: 150                // 稍微加高，讓 Dock 看起來更穩
    }

    // 調試輸出（可以在 QML 控制台看到）
    Component.onCompleted: {
        console.log("DashboardShell loaded")
        console.log("Waydroid available:", waydroidAvailable)
        if (waydroidAvailable) {
            console.log("Waydroid running:", Waydroid.running)
            console.log("Apps count:", Waydroid.appsModel.count)
            console.log("Apps available:", appsAvailable)
            
            // 監聽變化
            Waydroid.runningChanged.connect(function() {
                console.log("Waydroid running changed to:", Waydroid.running)
            })
            Waydroid.appsModel.countChanged.connect(function() {
                console.log("Apps count changed to:", Waydroid.appsModel.count)
                console.log("Apps available:", appsAvailable)
            })
        }
    }
}
