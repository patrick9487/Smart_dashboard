import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWayland.Compositor
import SmartDashboard 1.0
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
    
    // 當前嵌入的應用視窗嵌入器（視窗疊加模式）
    property var currentEmbedder: null
    
    // Compositor 模式相關屬性
    // 從 C++ 獲取 compositor 模式狀態（因為 QML 中沒有 qEnvironmentVariableIsSet）
    property bool compositorMode: typeof CompositorModeEnabled !== "undefined" ? CompositorModeEnabled : false
    property var currentSurface: null
    
    // 調試：顯示當前模式狀態（可在 UI 中顯示）
    property string modeStatus: compositorMode ? 
        ("Compositor 模式" + (compositorSurfaceModel.count > 0 ? " [有表面]" : " [無表面]")) : 
        "視窗疊加模式"
    
    // 用 ListModel 來保存 compositor surface，讓 Repeater 能正確感知 model 變化
    ListModel {
        id: compositorSurfaceModel
    }
    
    // 啟用 XDG Shell 協議，讓 Waydroid 等 xdg-shell client 可以連線
    // 這是讓 Waydroid 能正確創建視窗的關鍵！
    XdgShellHelper {
        id: xdgShellHelper
        compositor: waylandCompositor
    }
    
    // WaylandOutput - 連接到我們的 ApplicationWindow
    WaylandOutput {
        id: waylandOutput
        compositor: waylandCompositor
        sizeFollowsWindow: true
        window: window  // 連接到 ApplicationWindow
    }

    // ================== Wayland 輸入（關鍵） ==================
    // Linux 上某些 Qt build 的 QML WaylandSeat 是「不可在 QML 建立」的（會報 Type cannot be created）。
    // 我們改由 C++（XdgShellHelper）建立 QWaylandSeat，並透過 xdgShellHelper.seat 暴露給 QML。
    // 這裡用 WaylandCursorItem 做 software cursor（A/B 驗證硬體 cursor plane 閃爍）。
    WaylandCursorItem {
        id: softwareCursor
        // 有些 Qt 版本的 WaylandCursorItem 不需要/不支援 seat 綁定；保守起見只顯示即可
        visible: compositorMode
    }
    
    // Wayland Compositor（使用 QML 的 WaylandCompositor，參考 dashboard_compositor 專案）
    WaylandCompositor {
        id: waylandCompositor
        socketName: "wayland-smartdashboard-0"
        
        // 監聯表面創建
        onSurfaceCreated: function(surface) {
            console.log("🔵 WaylandCompositor: New surface created")
            console.log("  Surface object:", surface)
            console.log("  Current compositorSurfaceModel count:", compositorSurfaceModel.count)
            
            // 檢查是否已經存在（避免重複）
            for (var i = 0; i < compositorSurfaceModel.count; i++) {
                if (compositorSurfaceModel.get(i).surface === surface) {
                    console.log("  Surface already exists, skipping")
                    return
                }
            }
            
            compositorSurfaceModel.append({ surface: surface })
            console.log("  After append, compositorSurfaceModel count:", compositorSurfaceModel.count)
            
            // 監聽 surface 銷毀事件
            surface.surfaceDestroyed.connect(function() {
                console.log("🔴 WaylandCompositor: Surface destroyed")
                for (var i = 0; i < compositorSurfaceModel.count; i++) {
                    if (compositorSurfaceModel.get(i).surface === surface) {
                        compositorSurfaceModel.remove(i)
                        console.log("  Removed from model, new count:", compositorSurfaceModel.count)
                        break
                    }
                }
                // 如果當前表面被銷毀，清除引用
                if (currentSurface === surface) {
                    currentSurface = null
                }
            })
            
            // 如果還沒有當前表面，設置第一個表面為當前表面
            if (!currentSurface) {
                currentSurface = surface
                console.log("  Set as current surface")
            }
        }
    }


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
        z: 30
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 5
    }

    // 調試：顯示當前模式（右上角）
    Text {
        id: modeIndicator
        z: 30
        anchors.right: parent.right
        anchors.rightMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 5
        text: modeStatus
        color: compositorMode ? "#4a9eff" : "#f7b35a"
        font.pixelSize: 12
        visible: true  // 可以設置為 false 來隱藏
    }

    // 底部狀態列（Waydroid 有 app 時隱藏）
    StatusBarWidget {
        id: statusBar
        z: 30
        visible: !appsAvailable
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    // ODO：貼在狀態列上（Waydroid 有 app 時隱藏）
    OdometerWidget {
        id: odometer
        z: 30
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
        z: 30
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
        z: 30
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
        z: 30
        anchors.verticalCenter: speed.verticalCenter
        height: speed.height
        width: Math.min(height * 0.38, window.width * 0.14)
        anchors.left: speed.right

        // 初始時略微向左插入速度區塊，畫面變寬時慢慢往右「拉開」一點距離
        property real baseOverlap: -width * 0.55                      // 基本向左重疊量（負值越大越往左）
        property real extraGap: Math.min(40, Math.max(0, (window.width - 1100) * 0.06))
        anchors.leftMargin: baseOverlap + extraGap
    }

    // ================== 底部 App Dock（Waydroid 有 app 時顯示） ==================
    AppDock {
        id: appDock
        z: 40
        visible: appsAvailable  // 只有在有 app 時才顯示
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 0
        anchors.rightMargin: 0
        anchors.bottomMargin: 16   // 與視窗底部保留一點空間
        height: 150                // 稍微加高，讓 Dock 看起來更穩
        
        // 當點擊 AppIcon 時，創建嵌入器或查找表面
        onAppClicked: function(packageName) {
            console.log("========================================")
            console.log("DashboardShell: App clicked, package:", packageName)
            console.log("DashboardShell: Compositor mode:", compositorMode)
            console.log("DashboardShell: Current surface count:", compositorSurfaceModel.count)
            
            if (compositorMode && waylandCompositor) {
                // Compositor 模式：啟動應用並等待表面創建
                console.log("DashboardShell: Using compositor mode, launching app:", packageName)
                
                // 啟動應用（應用會連接到我們的 compositor）
                if (waydroidAvailable) {
                    console.log("DashboardShell: Launching app via Waydroid.launchApp...")
                    Waydroid.launchApp(packageName)
                    console.log("DashboardShell: launchApp called, waiting for surface...")
                    
                    // 等待表面創建（通過監聽 surfaceCreated 信號）
                    // 當表面創建時，waylandCompositor 會自動處理
                }
            } else {
                // 視窗疊加模式
                console.log("DashboardShell: Using window overlay mode")
                if (waydroidAvailable) {
                    // 如果已經有嵌入器，先停止它
                    if (currentEmbedder) {
                        currentEmbedder.stopEmbedding()
                        currentEmbedder = null
                    }
                    
                    // 創建新的嵌入器
                    currentEmbedder = Waydroid.createWindowEmbedder(packageName)
                    if (currentEmbedder) {
                        console.log("DashboardShell: Embedder created successfully, starting embedding...")
                        // 啟動嵌入過程（這會啟動應用並開始查找視窗）
                        currentEmbedder.startEmbedding()
                    } else {
                        console.error("DashboardShell: Failed to create embedder")
                    }
                }
            }
        }
    }
    
    // ================== 嵌入的應用視窗區域 ==================
    // Compositor 模式：顯示 Wayland surface（底層），儀表 UI 疊在上面
    Item {
        id: appArea
        anchors.fill: parent
        visible: compositorMode && compositorSurfaceModel.count > 0
        z: 5
        clip: true

        Repeater {
            model: compositorSurfaceModel
            delegate: WaylandQuickItem {
                surface: model.surface
                anchors.fill: parent
                focusOnClick: true

                Component.onCompleted: {
                    console.log("WaylandQuickItem created for surface:", model.surface)
                }
            }
        }
    }
    
    // 視窗疊加模式：當 compositor 模式未啟用時使用
    AppWindowEmbed {
        id: appWindowEmbed
        visible: !compositorMode && currentEmbedder !== null
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: timeWidget.bottom
        anchors.topMargin: 20
        anchors.bottom: speed.top
        anchors.bottomMargin: 20
        anchors.leftMargin: 40
        anchors.rightMargin: 40
        embedder: currentEmbedder
    }

    // 調試輸出（可以在 QML 控制台看到）
    Component.onCompleted: {
        console.log("========================================")
        console.log("DashboardShell loaded")
        console.log("Waydroid available:", waydroidAvailable)
        console.log("Compositor mode:", compositorMode)
        console.log("Compositor object:", waylandCompositor ? "exists" : "undefined")
        
        if (compositorMode && waylandCompositor) {
            console.log("✓ Compositor 模式已啟用")
            console.log("Compositor Socket Name:", waylandCompositor.socketName)
            console.log("Compositor created, waiting for surfaces...")
        } else {
            console.log("⚠ Compositor 模式未啟用 - 使用視窗疊加模式")
            if (!compositorMode) {
                console.log("提示：設置環境變量 SMART_DASHBOARD_COMPOSITOR=1 來啟用")
            }
        }
        
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
        console.log("========================================")
    }
}
