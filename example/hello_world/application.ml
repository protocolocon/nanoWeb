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
            beginPath(), roundedRect(x+2,   y+2,   w-4, h-4, 1), fillVertGrad(y, h, background%60, background%36), fill(),
            beginPath(), roundedRect(x+1.5, y+2.5, w-3, h-4, 2), strokeColor(background%72), stroke(),
            beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-4, 2), strokeColor(background%22), stroke(),
            font("Roboto-Regular.ttf", 20),
            fillColor(foreground%0),   text(0, 1, "Hello world!"),
            fillColor(foreground%200), text(0, 0, "Hello world!")
        ]
        onRenderActive: [
            strokeWidth(1),
            beginPath(), roundedRect(x+2,   y+2,   w-4, h-4, 1), fillVertGrad(y, h, background%40, background%24), fill(),
            beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-3, 2), strokeColor(background%72), stroke(),
            beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-4, 2), strokeColor(background%22), stroke(),
            font("Roboto-Regular.ttf", 20),
            fillColor(foreground%0),   text(0,  0, "Hello world!"),
            fillColor(foreground%160), text(0, -1, "Hello world!")
        ]
        foreground: "#808080a0"
        background: "#808080"
    }

    Widget {
        define: Button2
        width: 260
        height: 70
        onRender: [
            strokeWidth(2),
            beginPath(), roundedRect(x+4,   y+4,   w-8, h-8, 2), fillVertGrad(y, h, background%60, background%36), fill(),
            beginPath(), roundedRect(x+3,   y+5,   w-6, h-8, 4), strokeColor(background%72), stroke(),
            beginPath(), roundedRect(x+3,   y+3,   w-6, h-8, 4), strokeColor(background%22), stroke(),
            font("Roboto-Bold.ttf", 40),
            fillColor(foreground%0),   text(0, 2, "Hello world!"),
            fillColor(foreground%200), text(0, 0, "Hello world!")
        ]
        onRenderActive: [
            strokeWidth(2),
            beginPath(), roundedRect(x+4,   y+4,   w-8, h-8, 2), fillVertGrad(y, h, background%40, background%24), fill(),
            beginPath(), roundedRect(x+3,   y+3,   w-6, h-6, 4), strokeColor(background%72), stroke(),
            beginPath(), roundedRect(x+3,   y+3,   w-6, h-8, 4), strokeColor(background%22), stroke(),
            font("Roboto-Bold.ttf", 40),
            fillColor(foreground%0),   text(0,  0, "Hello world!"),
            fillColor(foreground%160), text(0, -2, "Hello world!")
        ]
        foreground: "#808080a0"
        background: "#808080"
    }


    LayoutHor {
        Button {
            id: test
            background: "#909090"
            onEnter: set(test, background, "#888898")
            onLeave: set(test, background, "#909090")
            onClick: toggleVisible(test3)
        }
        Button2 {
            id: test2
            background: "#909090"
            onEnter: set(test2, background, "#888898")
            onLeave: set(test2, background, "#909090")
            onClick: toggleVisible(test4)
        }
        Button {
            id: test3
        }
        Button {
            id: test4
        }
    }
}
