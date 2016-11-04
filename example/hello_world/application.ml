// Example application description
Application {
    background: "#4d4d4d"

    LayoutHor {
        Widget {
            id: buttonTest
            width: 100
            height: 32
            onEnter: [
                log("enter"),
                toggleVisible(buttonTest2)
            ]
            onLeave: log("leave")
            onRender: [
                strokeWidth(1),
                beginPath(), roundedRect(x+2,   y+2,   w-4, h-4, 1), fillVertGrad(y, h, "#605040", "#3c3630"),
                beginPath(), roundedRect(x+1.5, y+2.5, w-3, h-4, 2), strokeColor("#806c5c"), stroke(),
                beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-4, 2), strokeColor("#28201d"), stroke()
            ]
        }
        Widget {
            id: buttonTest2
            width: 200%
            height: 40
            onEnter: log("enter 2")
            onClick: toggleVisible(buttonTest3)
            onRender: [
                strokeWidth(1),
                beginPath(), roundedRect(x+2,   y+2,   w-4, h-4, 1), fillVertGrad(y, h, "#405060", "#30363c"),
                beginPath(), roundedRect(x+1.5, y+2.5, w-3, h-4, 2), strokeColor("#5c6c80"), stroke(),
                beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-4, 2), strokeColor("#1d2028"), stroke()
            ]
        }
        Widget {
            id: buttonTest3
            width: 200%
            height: 80
            onClick: toggleVisible(buttonTest)
            onRender: [
                strokeWidth(1),
                beginPath(), roundedRect(x+2,   y+2,   w-4, h-4, 1), fillVertGrad(y, h, "#506040", "#363c30"),
                beginPath(), roundedRect(x+1.5, y+2.5, w-3, h-4, 2), strokeColor("#6c805c"), stroke(),
                beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-4, 2), strokeColor("#20281d"), stroke()
            ]
        }
    }
}
