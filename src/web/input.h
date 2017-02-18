/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "vector.h"

struct GLFWwindow;

namespace webui {

    class Widget;

    struct Input {
        static void init();
        static bool refresh(); // returns true if input did potential changes in application
        static bool refreshStack(Widget* widget);

        static V2f cursor;                 // mouse cursor position
        static V2f cursorLeftPress;        // mouse cursor position when left button was pressed
        static bool updateCalled;          // if app.update() was called during refresh
        static bool updateModifications;   // if any app.update() returned true => app has to check for layout modifications
        static bool mouseButtonAction;     // if there are mouse button actions
        static bool keyboardAction;        // if there is keyboard action
        static int mouseButton;            // glfw enum
        static int mouseAction;            // glfw enum
        static int currentMods;            // glfw
        static Widget* mouseButtonWidget;  // the widget taking the press action
    };

}
