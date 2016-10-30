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
    class Application;

    struct Input {
        static void init(GLFWwindow* win, Application* app);
        static bool refresh(GLFWwindow* win); // returns true if input did potential changes in application

        static V2s cursor;                 // mouse cursor position
        static bool updateCalled;          // if app.update() was called during refresh
        static bool updateModifications;   // if any app.update() returned true => app has to check for layout modifications
        static bool mouseButtonAction;     // if there are mouse button actions
        static bool keyboardAction;        // if there is keyboard action
        static int mouseButton;            // glfw enum
        static int mouseAction;            // glfw enum
        static int currentMods;            // glfw
        static Widget* mouseButtonPress;   // the widget taking the press action
    };

}
