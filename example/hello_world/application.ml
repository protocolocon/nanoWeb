// Example application description
Application {
    background: "#4d4d4d"

    // define a widget button to be used later
    Widget {
        define: ButtonLevel1
        width: 260
        height: 70
        onRender: [
            strokeWidth(2),
            beginPath(), roundedRect(x+4,   y+4,   w-8, h-8, 2), fillVertGrad(y, h, background%60, background%36), fill(),
            beginPath(), roundedRect(x+3,   y+5,   w-6, h-8, 4), strokeColor(background%72), stroke(),
            beginPath(), roundedRect(x+3,   y+3,   w-6, h-8, 4), strokeColor(background%22), stroke(),
            font("Roboto-Bold.ttf", 40),
            fillColor(foreground%0),   text(0, 2, text),
            fillColor(foreground%200), text(0, 0, text)
        ]
        onRenderActive: [
            strokeWidth(2),
            beginPath(), roundedRect(x+4,   y+4,   w-8, h-8, 2), fillVertGrad(y, h, background%40, background%24), fill(),
            beginPath(), roundedRect(x+3,   y+3,   w-6, h-6, 4), strokeColor(background%72), stroke(),
            beginPath(), roundedRect(x+3,   y+3,   w-6, h-8, 4), strokeColor(background%22), stroke(),
            font("Roboto-Bold.ttf", 40),
            fillColor(foreground%0),   text(0,  0, text),
            fillColor(foreground%160), text(0, -2, text)
        ]
        onEnter: set(self, background, "#888898c0")
        onLeave: set(self, background, "#909090c0")
        foreground: "#808080a0"
        background: "#808080c0"
    }

    Widget {
        define: ButtonLevel2
        width: 120
        height: 32
        onRender: [
            strokeWidth(1),
            beginPath(), roundedRect(x+2,   y+2,   w-4, h-4, 1), fillVertGrad(y, h, background%60, background%36), fill(),
            beginPath(), roundedRect(x+1.5, y+2.5, w-3, h-4, 2), strokeColor(background%72), stroke(),
            beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-4, 2), strokeColor(background%22), stroke(),
            font("Roboto-Regular.ttf", 20),
            fillColor(foreground%0),   text(0, 1, text),
            fillColor(foreground%200), text(0, 0, text)
        ]
        onRenderActive: [
            strokeWidth(1),
            beginPath(), roundedRect(x+2,   y+2,   w-4, h-4, 1), fillVertGrad(y, h, background%40, background%24), fill(),
            beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-3, 2), strokeColor(background%72), stroke(),
            beginPath(), roundedRect(x+1.5, y+1.5, w-3, h-4, 2), strokeColor(background%22), stroke(),
            font("Roboto-Regular.ttf", 20),
            fillColor(foreground%0),   text(0,  0, text),
            fillColor(foreground%160), text(0, -1, text)
        ]
        foreground: "#808080a0"
        background: "#808080c0"
    }

    ButtonLevel2 {
        define: ButtonLevel3
        width: 80
        height: 24
        background: "#a08080c0"
        text: "more"
    }

    // test template
    Timer {
        id: timer
        repeat: 1
        delay: 5000 // ms
        onTimeout: query("tree.ml", treeTemplate)
    }

    Template {
        id: treeTemplate
        ButtonLevel1 {
            id: @
            text: @
        }
    }

    // tree definition
    LayoutVer {
        id: root

        Template {
            ButtonLevel1 {
                id: @
                text: @
                onClick: toggleVisible(@)
            }
            LayoutVer {
                id: @
                height: 0%
                ButtonLevel2 {
                    id: @
                    text: @
                    onClick: toggleVisible(@)
                }
                LayoutVer {
                    id: @
                    height: 0%
                    ButtonLevel3 {
                        text: @
                    }
                }
            }
        }

        ButtonLevel1 {
            id: but1
            text: "level 1 first"
            onClick: toggleVisible(lay1)
        }
        LayoutVer {
            id: lay1
            height: 0%
            ButtonLevel2 {
                id: but1_1
                text: "level 2 first"
                onClick: toggleVisible(lay1_1)
            }
            LayoutVer {
                id: lay1_1
                height: 0%
                ButtonLevel3 { id: level1_1_1 }
                ButtonLevel3 { id: level1_1_2 }
                ButtonLevel3 { id: level1_1_3 }
            }
            ButtonLevel2 {
                id: but1_2
                text: "level 2 second"
                onClick: toggleVisible(lay1_2)
            }
            LayoutVer {
                id: lay1_2
                height: 0%
                ButtonLevel3 { id: level1_2_1 }
                ButtonLevel3 { id: level1_2_2 }
                ButtonLevel3 { id: level1_2_3 }
            }
            ButtonLevel2 {
                id: but1_3
                text: "level 2 third"
                onClick: toggleVisible(lay1_3)
            }
            LayoutVer {
                id: lay1_3
                height: 0%
                ButtonLevel3 { id: level1_3_1 }
                ButtonLevel3 { id: level1_3_2 }
                ButtonLevel3 { id: level1_3_3 }
            }
        }
        ButtonLevel1 {
            id: but2
            text: "level 1 second"
            onClick: toggleVisible(lay2)
        }
        LayoutVer {
            id: lay2
            height: 0%
            ButtonLevel2 {
                id: but2_1
                text: "level 2 first"
                onClick: toggleVisible(lay2_1)
            }
            LayoutVer {
                id: lay2_1
                height: 0%
                ButtonLevel3 { id: level2_1_1 }
                ButtonLevel3 { id: level2_1_2 }
                ButtonLevel3 { id: level2_1_3 }
            }
        }
        ButtonLevel1 {
            id: but3
            text: "level 1 third"
            onClick: toggleVisible(lay3)
        }
        LayoutVer {
            id: lay3
            height: 0%
            ButtonLevel2 {
                id: but3_1
                text: "level 2 first"
                onClick: toggleVisible(lay3_1)
            }
            LayoutVer {
                id: lay3_1
                height: 0%
                ButtonLevel3 { id: level3_1_1 }
                ButtonLevel3 { id: level3_1_2 }
                ButtonLevel3 { id: level3_1_3 }
            }
            ButtonLevel2 {
                id: but3_2
                text: "level 2 second"
                onClick: toggleVisible(lay3_2)
            }
            LayoutVer {
                id: lay3_2
                height: 0%
                ButtonLevel3 { id: level3_2_1 }
                ButtonLevel3 { id: level3_2_2 }
                ButtonLevel3 { id: level3_2_3 }
            }
            ButtonLevel2 {
                id: but3_3
                text: "level 2 third"
                onClick: toggleVisible(lay3_3)
            }
            LayoutVer {
                id: lay3_3
                height: 0%
                ButtonLevel3 { id: level3_3_1 }
                ButtonLevel3 { id: level3_3_2 }
                ButtonLevel3 { id: level3_3_3 }
            }
        }
    }
}
