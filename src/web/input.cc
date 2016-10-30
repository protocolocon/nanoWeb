/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "input.h"
#include "application.h"
#include "compatibility.h"

using namespace std;
using namespace webui;

namespace {

    // this is required because glfw does not provide a fast way for callbacks to access context
    Application* app;

}

namespace webui {

    void Input::init(GLFWwindow* win, Application* app_) {
        app = app_;
        glfwPollEvents();
        glfwSetMouseButtonCallback(win, [](GLFWwindow* win, int button, int action, int mods) {
                mouseButtonAction = true;
                mouseButton = button;
                mouseAction = action;
                currentMods = mods;
                updateCalled = true;
                updateModifications |= app->update();
                mouseButtonAction = false;
            });
    }

    bool Input::refresh(GLFWwindow* win) {
        // get cursor position
        double mx, my;
        glfwGetCursorPos(win, &mx, &my);
        cursor = V2s(short(mx), short(my));
        // poll events
        glfwPollEvents(); // in the browser this does nothing, as events are processed asynchronously
        // call update if required
        if (!updateCalled && (cursor.x || cursor.y)) // in the browser, when cursor is outside, cursor is (0, 0)
            updateModifications |= app->update();
        bool ret(updateModifications);
        updateCalled = updateModifications = false;
        return ret;
    }

    V2s Input::cursor;
    bool Input::updateCalled;
    bool Input::updateModifications;
    bool Input::mouseButtonAction;
    bool Input::keyboardAction;
    int Input::mouseButton;
    int Input::mouseAction;
    int Input::currentMods;
    Widget* Input::mouseButtonPress;

}
