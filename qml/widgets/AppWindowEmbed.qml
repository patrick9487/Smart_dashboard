import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import SmartDashboard 1.0

/**
 * AppWindowEmbed
 * 
 * 用於在 QML 中顯示嵌入的 Waydroid 應用視窗
 * 
 * 使用方式：
 * - 設置 embedder 屬性為 WaydroidWindowEmbedder 實例
 * - 當視窗嵌入成功後，會自動顯示
 */
Item {
    id: root
    
    property var embedder: null
    property bool showPlaceholder: embedder ? !embedder.embedded : true
    
    // 佔位符：當視窗尚未嵌入時顯示
    Rectangle {
        id: placeholder
        anchors.fill: parent
        visible: showPlaceholder
        color: "#1a1a1a"
        border.color: "#333333"
        border.width: 2
        radius: 8
        
        Column {
            anchors.centerIn: parent
            spacing: 16
            
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: embedder && embedder.packageName ? 
                      qsTr("載入應用: %1").arg(embedder.packageName) : 
                      qsTr("等待應用啟動...")
                color: "#888888"
                font.pixelSize: 16
            }
            
            // 載入動畫
            Item {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 40
                height: 40
                
                Rectangle {
                    id: spinner
                    anchors.centerIn: parent
                    width: 30
                    height: 30
                    radius: width / 2
                    color: "transparent"
                    border.color: "#4a9eff"
                    border.width: 3
                    
                    RotationAnimation on rotation {
                        from: 0
                        to: 360
                        duration: 1000
                        loops: Animation.Infinite
                    }
                }
            }
        }
    }
    
    // 嵌入的視窗容器
    WindowEmbedItem {
        id: windowEmbedItem
        anchors.fill: parent
        visible: !showPlaceholder && embedder && embedder.embeddedWindow
        window: embedder && embedder.embedded ? embedder.embeddedWindow : null
    }
    
    // 監聽嵌入狀態變化
    Connections {
        target: embedder
        function onEmbeddedChanged() {
            console.log("AppWindowEmbed: embedded changed to", embedder ? embedder.embedded : false)
            if (embedder && embedder.embedded) {
                embedder.startEmbedding()
            }
        }
        function onEmbeddedWindowChanged() {
            console.log("AppWindowEmbed: embedded window changed")
        }
    }
    
    // 當 embedder 改變時，自動開始嵌入
    onEmbedderChanged: {
        if (embedder && embedder.packageName) {
            embedder.startEmbedding()
        }
    }
}

