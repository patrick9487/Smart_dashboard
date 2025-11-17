import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: "#0c0d11"
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#121318" }
            GradientStop { position: 0.6; color: "#0c0d11" }
            GradientStop { position: 1.0; color: "#08090d" }
        }
    }

    Row {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 32
        spacing: parent.width - 320

        Text {
            text: "00 : 00"
            font.pixelSize: 36
            color: "#DDE2E6"
            font.family: "Helvetica"
            font.bold: true
        }

        Row {
            spacing: 18
            anchors.verticalCenter: parent.verticalCenter

            Rectangle {
                width: 64
                height: 36
                radius: 6
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#f4f7fa" }
                    GradientStop { position: 1.0; color: "#d3d6db" }
                }
                border.color: "#16171A"
                border.width: 1.2
                Text {
                    anchors.centerIn: parent
                    text: "App"
                    color: "#1a1d22"
                    font.pixelSize: 18
                    font.bold: true
                }
            }

            Rectangle {
                width: 40
                height: 22
                radius: 4
                border.color: "#f4f7fa"
                border.width: 2
                color: "transparent"

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 4
                    width: parent.width - 12
                    height: parent.height - 8
                    radius: 2
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#f4f7fa" }
                        GradientStop { position: 1.0; color: "#d0d2d6" }
                    }
                }
            }
        }
    }

    Canvas {
        id: centralFrame
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: 460
        height: 260
        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)
            ctx.lineWidth = 6
            ctx.strokeStyle = "#cdd4da"
            ctx.lineCap = "round"
            ctx.beginPath()
            var top = 12
            var bottom = height - 12
            var leftInner = 80
            var rightInner = width - 80
            ctx.moveTo(leftInner, top)
            ctx.lineTo(rightInner, top)
            ctx.lineTo(width - 16, height / 2)
            ctx.lineTo(rightInner, bottom)
            ctx.lineTo(leftInner, bottom)
            ctx.lineTo(16, height / 2)
            ctx.closePath()
            ctx.stroke()
        }
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        Text {
            text: "0"
            color: "#FFFFFF"
            font.pixelSize: 168
            font.bold: true
            font.family: "Helvetica"
        }

        Text {
            text: "km/h"
            color: "#d5d8dc"
            font.pixelSize: 28
            font.letterSpacing: 2
        }
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.verticalCenter
        anchors.topMargin: 140
        spacing: 8
        width: 220

        Text {
            text: "ODO"
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#b5bbc4"
            font.pixelSize: 22
            font.letterSpacing: 4
        }

        Text {
            text: "888888 km"
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#eef2f5"
            font.pixelSize: 32
            font.bold: true
        }
    }

    Rectangle {
        width: 84
        height: 84
        radius: width / 2
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: -220
        anchors.verticalCenter: parent.verticalCenter
        color: "#ffffff"
        border.color: "#ff4040"
        border.width: 8
        opacity: 0.95

        Text {
            anchors.centerIn: parent
            text: "60"
            font.pixelSize: 32
            font.bold: true
            color: "#20242c"
        }
    }

    Item {
        id: leftScale
        width: 240
        height: 320
        anchors.left: parent.left
        anchors.leftMargin: 40
        anchors.verticalCenter: parent.verticalCenter

        Canvas {
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()
                ctx.clearRect(0, 0, width, height)

                var segments = 8
                for (var i = 0; i < segments; ++i) {
                    var top = 20 + i * 36
                    var bottom = top + 30
                    ctx.beginPath()
                    ctx.moveTo(width * 0.18, bottom)
                    ctx.lineTo(width * 0.65, bottom - 10)
                    ctx.lineTo(width * 0.92, bottom - 30)
                    ctx.lineTo(width * 0.45, top - 6)
                    ctx.closePath()

                    var grad = ctx.createLinearGradient(width * 0.18, top, width * 0.9, bottom)
                    grad.addColorStop(0, "rgba(255, 255, 200, " + (0.15 + i / (segments * 1.4)) + ")")
                    grad.addColorStop(0.6, "rgba(255, 245, 180, " + (0.35 + i / (segments * 1.6)) + ")")
                    grad.addColorStop(1, "rgba(210, 180, 60, " + (0.2 + i / (segments * 1.8)) + ")")
                    ctx.fillStyle = grad
                    ctx.fill()

                    ctx.strokeStyle = "rgba(255,255,255,0.25)"
                    ctx.lineWidth = 1.2
                    ctx.stroke()
                }
            }
        }

        Column {
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.verticalCenter: parent.verticalCenter
            spacing: 14
            Repeater {
                model: ["10", "8", "6", "4", "2", "0"]
                delegate: Text {
                    text: modelData
                    color: model.index === 0 ? "#ff6a5a" : "#f0f4ff"
                    font.pixelSize: 24
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }

        Text {
            text: "x1000 r/min"
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 12
            anchors.bottomMargin: -12
            color: "#9aa3b2"
            font.pixelSize: 18
        }
    }

    Item {
        id: rightScale
        width: 240
        height: 320
        anchors.right: parent.right
        anchors.rightMargin: 40
        anchors.verticalCenter: parent.verticalCenter

        Canvas {
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()
                ctx.clearRect(0, 0, width, height)

                var segments = 7
                for (var i = 0; i < segments; ++i) {
                    var top = 26 + i * 38
                    var bottom = top + 32
                    ctx.beginPath()
                    ctx.moveTo(width * 0.82, bottom)
                    ctx.lineTo(width * 0.35, bottom - 10)
                    ctx.lineTo(width * 0.08, bottom - 30)
                    ctx.lineTo(width * 0.55, top - 6)
                    ctx.closePath()

                    var grad = ctx.createLinearGradient(width * 0.82, top, width * 0.12, bottom)
                    grad.addColorStop(0, "rgba(255, 255, 200, " + (0.18 + (segments - i) / (segments * 1.6)) + ")")
                    grad.addColorStop(0.6, "rgba(255, 245, 180, " + (0.36 + (segments - i) / (segments * 1.8)) + ")")
                    grad.addColorStop(1, "rgba(210, 180, 60, " + (0.22 + (segments - i) / (segments * 2.0)) + ")")
                    ctx.fillStyle = grad
                    ctx.fill()

                    ctx.strokeStyle = "rgba(255,255,255,0.2)"
                    ctx.lineWidth = 1.1
                    ctx.stroke()
                }
            }
        }

        Column {
            anchors.right: parent.right
            anchors.rightMargin: 6
            anchors.verticalCenter: parent.verticalCenter
            spacing: 46
            Repeater {
                model: ["F", "", "", "", "E"]
                delegate: Text {
                    visible: modelData !== ""
                    text: modelData
                    color: "#f0f4ff"
                    font.pixelSize: 28
                    font.bold: true
                }
            }
        }

        Rectangle {
            width: 32
            height: 32
            radius: 6
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            color: "transparent"
            border.color: "#f0f4ff"
            border.width: 2

            Rectangle {
                anchors.centerIn: parent
                width: 16
                height: 24
                color: "#f0f4ff"
                radius: 2
            }
        }

        Text {
            text: "Fuel"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: -18
            anchors.bottomMargin: -12
            color: "#9aa3b2"
            font.pixelSize: 18
        }
    }

    Row {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 36
        spacing: parent.width - 360

        Text {
            text: "VVA"
            color: "#f0f4ff"
            font.pixelSize: 26
            font.bold: true
        }

        Text {
            text: "TCS ON"
            color: "#f0f4ff"
            font.pixelSize: 26
            font.bold: true
        }
    }
}
