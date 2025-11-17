import QtQuick
import QtQuick.Shapes

Item {
    id: root
    implicitWidth: 180
    implicitHeight: 260

    property real topMargin: height * 0.03
    property real bottomMargin: height * 0.03
    property real gap: height * 0.015

    // 這邊 6 段都同高 → 6*barH + 5*gap + top + bottom = height
    property real barH: (height - topMargin - bottomMargin - (5 * gap)) / 6

    property real barW: width * 0.3
    property real skew: barH * 0.7
    property real textGap: width * 0.03

    property real baseX0: width * 0.25

    function barHeight(i) { return barH }
    function barSkew(i)   { return skew }

    function barTopY(i) {
        var y = topMargin
        for (var k = 0; k < i; k++)
            y += barH + gap
        return y
    }

    // 跟之前一樣：上半往右，下半往左，第 4 段直接接第 3 段
    function barBaseX(i) {
        if (i === 0) return baseX0

        var prev = i - 1
        var prevX = barBaseX(prev)
        var prevS = barSkew(prev)
        var prevBottomX = (prev < 3)
                ? (prevX + prevS)
                : (prevX - prevS)

        var slope = prevS / barH
        var horizGap = slope * gap

        if (i === 3)
            return prevBottomX

        return (prev < 3)
               ? (prevBottomX + horizGap)
               : (prevBottomX - horizGap)
    }

    // 條本體
    Repeater {
        model: 6
        delegate: Shape {
            anchors.fill: parent
            property int idx: index
            property real topY: root.barTopY(idx)
            property real baseX: root.barBaseX(idx)
            property real s: root.barSkew(idx)

            ShapePath {
                strokeWidth: 0
                fillColor: Qt.rgba(1, 0.96, 0.75, 1)

                startX: baseX; startY: topY
                PathLine { x: baseX + root.barW; y: topY }
                PathLine {
                    x: (idx < 3)
                        ? (baseX + root.barW + s)
                        : (baseX + root.barW - s)
                    y: topY + root.barH
                }
                PathLine {
                    x: (idx < 3)
                        ? (baseX + s)
                        : (baseX - s)
                    y: topY + root.barH
                }
                PathLine { x: baseX; y: topY }
            }
        }
    }

    // F / E 字
    Repeater {
        model: 6
        delegate: Item {
            width: 50; height: 20

            property int idx: index
            property real topY: root.barTopY(idx)
            property real baseX: root.barBaseX(idx)
            property real s: root.barSkew(idx)

            Text {
                visible: idx === 0 || idx === 5
                text: idx === 0 ? "-F" : "-E"
                color: "white"
                font.pixelSize: root.height * 0.045

                x: (idx === 0)
                   ? (baseX + root.barW + s + root.textGap)
                   : (baseX + root.barW - s + root.textGap)

                y: (idx === 0)
                   ? (topY - height * 0.5)
                   : (topY + root.barH - height * 0.6)
            }
        }
    }

    // 油槍
    Text {
        text: "\u26FD"
        color: "white"
        font.pixelSize: root.height * 0.07
        property real y3: root.barTopY(2)
        x: root.barBaseX(2) + root.barW + root.textGap * 9
        y: y3 + root.barH + root.gap / 2 - height / 2
    }
}
