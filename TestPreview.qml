// TestPreview.qml
import QtQuick
import QtQuick.Window
import "widgets"

Window {
    id: root
    visible: true
    color: "#1e1e1e"
    title: "Responsive Tachometer (Preview Mode Compatible)"

    width: 600
    height: 400

    // ---- 偵測 Preview 視窗尺寸變化 ----
    property int lastW: 0
    property int lastH: 0
    Timer {
        interval: 200
        running: true
        repeat: true
        onTriggered: {
            if (root.width !== root.lastW || root.height !== root.lastH) {
                root.lastW = root.width
                root.lastH = root.height
                tach.requestPaint()
            }
        }
    }

    // ---- Tachometer Widget ----
    TachometerWidget {
        id: tach
        anchors.centerIn: parent
        width: parent.width * 0.6
        height: parent.height * 0.8
    }

    // ---- 顯示當前尺寸（方便測試） ----
    Text {
        text: "Preview Size: " + root.width + " × " + root.height
        color: "#aaaaaa"
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10
        font.pixelSize: 14
    }
}
