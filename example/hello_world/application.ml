// Example application description
Application {
    background: "#4d4d4d"

    // define a widget button to be used later
    Widget {
        define: Button
        width: 100
        height: 32
        onRender: [
            strokeWidth(1),
            beginPath(), roundedRect(x+2,   y+2,   w-4, h-4, 1), fillVertGrad(y, h, background, background%60), fill(),
            beginPath(), roundedRect(x+1.5, y+2.5, w-3, h-4, 2), strokeColor(background%120), stroke(),
            beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-4, 2), strokeColor(background%30), stroke(),
            fillColor(foreground), font("Roboto-Regular.ttf", 22), text("hello world!")
        ]
        foreground: "#d0d0d0"
        background: "#806050"
    }


    LayoutHor {
        Button {
            background: "#808050"
        }
        Button {
            background: "#808080"
        }
        Button {
            background: "#804050"
            foreground: "#d04040"
        }
        Button {
            background: "#308050"
        }

        Widget {
            id: buttonTest
            width: 100
            height: 32
            onEnter: [
                log("enter"),
                toggleVisible(buttonTest2),
                set(buttonTest*, height, 120)
            ]
            onLeave: log("leave")
            onRender: [
                strokeWidth(1),
                beginPath(), roundedRect(x+2,   y+2,   w-4, h-4, 1), fillVertGrad(y, h, background%100, background%60), fill(),
                beginPath(), roundedRect(x+1.5, y+2.5, w-3, h-4, 2), strokeColor(background%120), stroke(),
                beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-4, 2), strokeColor(background%30), stroke()
            ]
            background: "#806050"
        }
        Widget {
            id: buttonTest2
            width: 200%
            height: 40
            onEnter: log("enter 2")
            onClick: toggleVisible(buttonTest3)
            onRender: [
                strokeWidth(1),
                beginPath(), roundedRect(x+2,   y+2,   w-4, h-4, 1), fillVertGrad(y, h, "#405060", "#30363c"), fill(),
                beginPath(), roundedRect(x+1.5, y+2.5, w-3, h-4, 2), strokeColor("#5c6c80"), stroke(),
                beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-4, 2), strokeColor("#1d2028"), stroke()
            ]
            onEnter: set(buttonTest, background, "#506080")
            onLeave: set(buttonTest, background, "#806050")
        }
        Widget {
            id: buttonTest3
            width: 200%
            height: 80
            onClick: toggleVisible(buttonTest)
            onRender: [
                strokeWidth(1),
                beginPath(), roundedRect(x+2,   y+2,   w-4, h-4, 1), fillVertGrad(y, h, "#506040", "#363c30"), fill(),
                beginPath(), roundedRect(x+1.5, y+2.5, w-3, h-4, 2), strokeColor("#6c805c"), stroke(),
                beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-4, 2), strokeColor("#20281d"), stroke()
            ]
        }
    }
}
