import QtQuick
import QtQuick.Shapes

Item {
    id: root
    implicitWidth: 180
    implicitHeight: 260

    // 上面留一點、下面要多留給文字
    property real topMargin: height * 0.03
    property real bottomTextSpace: height * 0.055   // 專門給 "x1000 r/min"
    property real bottomMargin: height * 0.02       // 字跟邊界的再留一點
    property real gap: height * 0.015

    // 4 段全高 + 2 段半高 = 5 段
    // ✅ 記得把 bottomTextSpace 也扣掉
    property real baseBarH: (height
                             - topMargin
                             - bottomTextSpace
                             - bottomMargin
                             - 5 * gap) / 5

    property real barW: width * 0.3
    property real skew: baseBarH * 0.7
    property real textGap: width * 0.02

    // 拿來量 "10-" 真的多寬
    Text {
        id: measureLabel
        text: "10-"
        font.pixelSize: root.height * 0.06
        visible: false
    }

    function barHeight(i) { return (i === 2 || i === 3) ? baseBarH / 2 : baseBarH }
    function barSkew(i)   { return (i === 2 || i === 3) ? skew / 2 : skew }

    // 前兩段往左的量
    property real s0: barSkew(0)
    property real s1: barSkew(1)
    property real gapProj0: (s0 / barHeight(0)) * gap
    property real gapProj1: (s1 / barHeight(1)) * gap

    property real leftMargin: width * 0.01

    // ✅ 這是你說的剛剛好那個版本
    property real baseX0: measureLabel.width
                          + s0 + gapProj0
                          + s1 + gapProj1

    // Y 疊上去
    function barTopY(i) {
        var y = topMargin
        for (var k = 0; k < i; k++)
            y += barHeight(k) + gap
        return y
    }

    // X：前三段往左，第四段接，後面往右
    function barBaseX(i) {
        if (i === 0)
            return baseX0

        var prev = i - 1
        var prevX = barBaseX(prev)
        var prevH = barHeight(prev)
        var prevS = barSkew(prev)

        var prevBottomX = (prev < 3)
                ? (prevX - prevS)
                : (prevX + prevS)

        var slope = prevS / prevH
        var horizGap = slope * gap

        if (i === 3)
            return prevBottomX

        return (prev < 3)
               ? (prevBottomX - horizGap)
               : (prevBottomX + horizGap)
    }

    // ========= 條本體 =========
    Repeater {
        model: 6
        delegate: Shape {
            anchors.fill: parent
            property int idx: index
            property real topY: root.barTopY(idx)
            property real h: root.barHeight(idx)
            property real s: root.barSkew(idx)
            property real baseX: root.barBaseX(idx)

            ShapePath {
                strokeWidth: 0
                fillColor: idx === 0 ? "#ff4444" : "#fff5c0"

                startX: baseX; startY: topY
                PathLine { x: baseX + root.barW; y: topY }
                PathLine {
                    x: (idx < 3)
                       ? (baseX + root.barW - s)
                       : (baseX + root.barW + s)
                    y: topY + h
                }
                PathLine {
                    x: (idx < 3)
                       ? (baseX - s)
                       : (baseX + s)
                    y: topY + h
                }
                PathLine { x: baseX; y: topY }
            }
        }
    }

    // ========= 左邊刻度 =========
    Repeater {
        model: ["10-", "8-", "6-", "4-", "2-", "0-"]
        delegate: Text {
            text: modelData
            color: "white"
            font.pixelSize: root.height * 0.06

            property int idx: index
            property real topY: root.barTopY(idx)
            property real h: root.barHeight(idx)
            property real s: root.barSkew(idx)
            property real baseX: root.barBaseX(idx)

            x: (idx < 3)
               ? (baseX - width - root.textGap)
               : (baseX + s - width - root.textGap)

            y: (idx < 3)
               ? (topY - height / 2)
               : (topY + h - height / 2)
        }
    }

    // ========= 底部單位 =========
    Text {
        text: "x1000 r/min"
        color: "white"
        font.pixelSize: root.height * 0.045

        // 直接用最下面那段的底 + 一點點 margin
        property real lastTop: root.barTopY(5)
        property real lastH: root.barHeight(5)
        property real lastS: root.barSkew(5)
        property real lastX: root.barBaseX(5)

        y: lastTop + lastH + root.height * 0.008   // 這裡你要高一點就再加
        x: lastX + lastS - root.width * 0.08
    }
}
