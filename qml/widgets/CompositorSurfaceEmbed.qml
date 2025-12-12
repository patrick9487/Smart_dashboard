import QtQuick 2.15
import QtQuick.Controls 2.15
import SmartDashboard 1.0

/**
 * CompositorSurfaceEmbed
 * 
 * 真正的 compositor 模式：使用 Wayland compositor 將視窗表面嵌入到 QML 場景中
 * 
 * 使用方式：
 * - 設置 surface 屬性為 QWaylandSurface 實例
 * - 表面內容會直接渲染到 QML 場景中
 */
Item {
    id: root
    
    property var surface: null
    property bool showPlaceholder: !surface || !surface.mapped
    
    // 佔位符：當表面尚未映射時顯示
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
                text: surface ? qsTr("等待表面映射...") : qsTr("沒有表面")
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
    
    // 真正的表面渲染（compositor 模式）
    SurfaceItem {
        id: surfaceItem
        anchors.fill: parent
        visible: !showPlaceholder && surface
        surface: root.surface
        
        // 當表面大小改變時，自動調整 Item 大小
        Connections {
            target: surface
            function onSizeChanged() {
                if (surface && surface.size.isValid()) {
                    root.width = surface.size.width
                    root.height = surface.size.height
                }
            }
        }
    }
    
    // 監聽表面狀態變化
    Connections {
        target: surface
        function onMappedChanged() {
            console.log("CompositorSurfaceEmbed: Surface mapped changed to", surface ? surface.mapped : false)
        }
    }
}

