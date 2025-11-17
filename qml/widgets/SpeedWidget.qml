import QtQuick

Item {
    id: root
    implicitWidth: 480
    implicitHeight: 272

    // 六角形比例控制（固定角度，不隨視窗變）
    property real topMargin:    height * 0.05
    property real bottomMargin: height * 0.05
    property real sideMargin:   width  * 0.03
    property real slopeRatio: 0.7

    property real midY: height / 2
    property real topY: topMargin
    property real bottomY: height - bottomMargin
    property real vDelta: midY - topY
    property real diagInset: vDelta * slopeRatio
    property real leftX:  sideMargin + diagInset
    property real rightX: width - sideMargin - diagInset

    // 六邊形外框
    Canvas {
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)
            ctx.lineWidth = Math.min(width, height) * 0.015
            ctx.strokeStyle = "rgba(210,215,225,0.9)"
            ctx.lineCap = "round"
            ctx.beginPath()
            ctx.moveTo(leftX, topY)
            ctx.lineTo(rightX, topY)
            ctx.lineTo(width - sideMargin, midY)
            ctx.lineTo(rightX, bottomY)
            ctx.lineTo(leftX, bottomY)
            ctx.lineTo(sideMargin, midY)
            ctx.closePath()
            ctx.stroke()
        }
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    // 主速度數字
    Text {
        id: speedValue
        text: "0"
        color: "white"
        font.bold: true
        // 數字更高一點
        font.pixelSize: root.height * 0.6
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    // 單位 km/h：與速度數字靠得近，非對六邊形底邊
    Text {
        id: unitLabel
        text: "km/h"
        color: "#d5d8dc"
        font.pixelSize: root.height * 0.09
        font.letterSpacing: 2
        anchors.horizontalCenter: speedValue.horizontalCenter
        anchors.top: speedValue.bottom
        anchors.topMargin: -root.height * 0.1  // 貼近數字上緣一點
    }

    // 速限標誌：大小隨寬高縮放（保持圓形比例）
    Item {
        id: speedLimit
        property real circleSize: Math.min(root.width, root.height) * 0.15
        width: circleSize
        height: circleSize
        anchors.verticalCenter: parent.verticalCenter
        x: sideMargin + circleSize * 0.5 + root.width * 0.01

        Rectangle {
            anchors.fill: parent
            radius: width / 2
            color: "white"
            border.color: "#ff3a3a"
            border.width: width * 0.12
        }

        Text {
            anchors.centerIn: parent
            text: "60"
            color: "#1c1f26"
            font.bold: true
            font.pixelSize: parent.height * 0.42
        }
    }
}

