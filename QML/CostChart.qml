import QtQuick

Item {
    id: root
    property var costData: costSource.costData

    onCostDataChanged: chartCanvas.requestPaint()

    Rectangle {
        anchors.fill: parent
        color: "#1e1e1e"
    }

    Text {
        id: title
        text: "Sherlog Builds — custo diário (USD)"
        color: "white"
        font.pixelSize: 22
        anchors.top: parent.top
        anchors.topMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Canvas {
        id: chartCanvas
        anchors.top: title.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 40

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)

            if (root.costData.length === 0)
                return

            var padding = 50
            var plotW = width - padding * 2
            var plotH = height - padding * 2

            var maxCost = 0
            var maxIdx = 0
            for (var i = 0; i < root.costData.length; i++) {
                if (root.costData[i].cost_usd > maxCost) {
                    maxCost = root.costData[i].cost_usd
                    maxIdx = i
                }
            }
            var axisMax = Math.max(maxCost * 1.15, 0.01)

            function xForIndex(idx) {
                return root.costData.length > 1
                    ? padding + (plotW * idx) / (root.costData.length - 1)
                    : padding
            }
            function yForCost(cost) {
                return padding + plotH - (plotH * cost) / axisMax
            }

            // eixos
            ctx.strokeStyle = "#888888"
            ctx.lineWidth = 1
            ctx.beginPath()
            ctx.moveTo(padding, padding)
            ctx.lineTo(padding, padding + plotH)
            ctx.lineTo(padding + plotW, padding + plotH)
            ctx.stroke()

            // grade + rótulos do eixo Y (em $)
            ctx.strokeStyle = "#3a3a3a"
            ctx.fillStyle = "#aaaaaa"
            ctx.font = "12px sans-serif"
            ctx.textAlign = "right"
            ctx.textBaseline = "middle"
            var ySteps = 4
            for (var s = 0; s <= ySteps; s++) {
                var value = axisMax * s / ySteps
                var y = yForCost(value)
                if (s > 0) {
                    ctx.beginPath()
                    ctx.moveTo(padding, y)
                    ctx.lineTo(padding + plotW, y)
                    ctx.stroke()
                }
                ctx.fillText("$" + value.toFixed(2), padding - 8, y)
            }

            // rótulos do eixo X (datas) — só alguns, pra não amontoar
            ctx.textAlign = "center"
            ctx.textBaseline = "top"
            var labelCount = Math.min(root.costData.length, 7)
            var step = Math.max(1, Math.floor((root.costData.length - 1) / (labelCount - 1 || 1)))
            for (var j = 0; j < root.costData.length; j += step)
                ctx.fillText(root.costData[j].date.substring(5), xForIndex(j), padding + plotH + 8)
            var lastIdx = root.costData.length - 1
            ctx.fillText(root.costData[lastIdx].date.substring(5), xForIndex(lastIdx), padding + plotH + 8)

            // linha
            ctx.strokeStyle = "#4fc3f7"
            ctx.lineWidth = 2
            ctx.beginPath()
            for (var k = 0; k < root.costData.length; k++) {
                var x = xForIndex(k)
                var yLine = yForCost(root.costData[k].cost_usd)
                if (k === 0)
                    ctx.moveTo(x, yLine)
                else
                    ctx.lineTo(x, yLine)
            }
            ctx.stroke()

            // pontos
            ctx.fillStyle = "#4fc3f7"
            for (var m = 0; m < root.costData.length; m++) {
                var px = xForIndex(m)
                var py = yForCost(root.costData[m].cost_usd)
                ctx.beginPath()
                ctx.arc(px, py, 3, 0, Math.PI * 2)
                ctx.fill()
            }

            // valor em destaque no pico e no ponto mais recente
            ctx.fillStyle = "#ffffff"
            ctx.font = "bold 13px sans-serif"
            ctx.textAlign = "center"
            ctx.textBaseline = "bottom"
            function labelPoint(idx) {
                ctx.fillText("$" + root.costData[idx].cost_usd.toFixed(2),
                             xForIndex(idx), yForCost(root.costData[idx].cost_usd) - 8)
            }
            labelPoint(maxIdx)
            if (lastIdx !== maxIdx)
                labelPoint(lastIdx)
        }
    }
}
