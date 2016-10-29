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
                toggle(buttonTest2)
            ]
        }
        Button {
            id: buttonTest2
            width: 128
            height: 70%
            color: "#00c080"
            onEnter: log("enter 2")
        }
        Button {
            id: buttonTest3
            width: 50%
            height: 60%
            color: "#00c0ff"
        }
    }
}
