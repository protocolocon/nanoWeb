// Example application description
Application {
    color: "#4d4d4d"

    LayoutHor {
        Button {
            id: buttonTest
            width: 25%
            height: 50%
            color: "#ffc000"
            onEnter: [
                log("enter"),
                toggleVisible(buttonTest2)
            ]
            onLeave: log("leave")
        }
        Button {
            id: buttonTest2
            width: 128
            height: 70%
            color: "#00c080"
            onEnter: log("enter 2")
            onClick: toggleVisible(buttonTest3)
        }
        Button {
            id: buttonTest3
            width: 50%
            height: 60%
            color: "#00c0ff"
        }
    }
}
