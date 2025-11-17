import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 800
    height: 480
    visible: true
    title: "Smart Dashboard"

    // 主框架：固定上方狀態列 + 中間內容區
    Column {
        anchors.fill: parent
        spacing: 0

        // 上方狀態列
        Rectangle {
            id: statusBar
            width: parent.width
            height: 60
            color: "#2C3E50"
            border.color: "#34495E"
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: "Smart Dashboard"
                font.pixelSize: 24
                font.bold: true
                color: "white"
            }
        }

        // 中間內容區（使用 Loader 動態載入主頁面）
        Rectangle {
            width: parent.width
            height: parent.height - statusBar.height
            color: "#101010"

            Loader {
                id: contentLoader
                anchors.fill: parent
                source: AppConfig.isLoaded ? AppConfig.homePage : ""
                
                onLoaded: {
                    console.log("Main page loaded successfully:", AppConfig.homePage)
                }
                
                onStatusChanged: {
                    if (status === Loader.Error) {
                        console.error("Failed to load page:", AppConfig.homePage)
                        console.error("Error:", sourceComponent ? sourceComponent.errorString() : "Unknown error")
                    }
                }
            }
        }
    }
}

