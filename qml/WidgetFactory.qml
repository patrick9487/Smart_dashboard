pragma Singleton
import QtQuick

QtObject {
    id: factory

    // 根據 type 創建對應的 widget 元件
    function createWidget(type, parent, x, y) {
        var component
        var widget
        var sourceUrl = ""
        
        // 使用完整的資源路徑（注意：資源路徑包含 qml/ 前綴）
        switch(type) {
            case "speed":
                sourceUrl = "qrc:/qt/qml/SmartDashboard/qml/widgets/SpeedWidget.qml"
                break
            case "fuel":
                sourceUrl = "qrc:/qt/qml/SmartDashboard/qml/widgets/FuelWidget.qml"
                break
            case "android-slot":
                sourceUrl = "qrc:/qt/qml/SmartDashboard/qml/widgets/AndroidSlot.qml"
                break
            default:
                console.warn("Unknown widget type:", type)
                return null
        }
        
        console.log("Creating widget from URL:", sourceUrl)

        component = Qt.createComponent(sourceUrl)
        
        if (component.status === Component.Ready) {
            widget = component.createObject(parent, {
                "x": x,
                "y": y
            })
            if (widget === null) {
                console.error("Failed to create widget:", type)
            } else {
                console.log("Widget created successfully:", type)
            }
        } else if (component.status === Component.Loading) {
            var createWidgetCallback = function() {
                if (component.status === Component.Ready) {
                    widget = component.createObject(parent, {
                        "x": x,
                        "y": y
                    })
                    if (widget === null) {
                        console.error("Failed to create widget:", type)
                    } else {
                        console.log("Widget created successfully (async):", type)
                    }
                } else if (component.status === Component.Error) {
                    console.error("Error loading widget:", component.errorString())
                }
            }
            component.statusChanged.connect(createWidgetCallback)
        } else if (component.status === Component.Error) {
            console.error("Error loading widget:", component.errorString(), "URL:", sourceUrl)
        }

        return widget
    }
}

