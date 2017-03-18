/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "input.h"
#include "widget.h"
#include "context.h"
#include "application.h"
#include "compatibility.h"

using namespace std;
using namespace webui;

namespace {

    const int HoverTimeMs = 500;
}

namespace webui {

    void Input::init() {
        auto* win(Context::render.getWin());
        glfwPollEvents();
        glfwSetMouseButtonCallback(win, [](GLFWwindow* win, int button, int action, int mods) {
                mouseButtonAction = true;
                mouseButton = button;
                mouseAction = action;
                currentMods = mods;
                updateCalled = true;
                if (Input::mouseButton == GLFW_MOUSE_BUTTON_LEFT && Input::mouseAction == GLFW_PRESS)
                    cursorLeftPress = cursor;
                updateModifications |= Context::app.update();
                if (action == GLFW_RELEASE) {
                    updateModifications |= refreshStack(mouseButtonWidget);
                    mouseButtonWidget = nullptr;
                }
                mouseButtonAction = false;
            });
        glfwSetKeyCallback(win, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                if (action == GLFW_PRESS) {
                    // exit
                    if (key == GLFW_KEY_ESCAPE && mods == GLFW_MOD_CONTROL)
                        cancelMainLoop();
                    // debug
                    DIAG(else if (key == GLFW_KEY_ESCAPE && mods == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT))
                             Context::app.dump());
                }
            });
        glfwSetScrollCallback(win, [](GLFWwindow* window, double xoff, double yoff) {
                scroll.x = xoff;
                scroll.y = yoff;
                scrollAction = updateCalled = true;
                updateModifications |= Context::app.update();
                scrollAction = false;
                scroll.x = scroll.y = 0.0f;
            });
    }

    bool Input::refresh() {
        auto* win(Context::render.getWin());

        // poll events
        glfwPollEvents(); // in the browser this does nothing, as events are processed asynchronously

        bool ret(updateModifications);

        // get cursor position
        double mx, my;
        glfwGetCursorPos(win, &mx, &my);
        V2f newCursor = V2f(mx, my);
        if (newCursor != cursor) { // do nothing if mouse is not moved
            cursor = newCursor;
            // drag & drop
            if (mouseButtonWidget) ret |= refreshStack(mouseButtonWidget);
            // call update if required
            if (!updateCalled && (cursor.x || cursor.y)) // in the browser, when cursor is outside, cursor is (0, 0)
                ret |= Context::app.update();
            // hover
            if (hoverWidget != Context::hoverWidget) {
                if (hoverWidget) ret = true;               // make hover effect disappear immediately
                hoverTime = ctx.getTimeMs() + HoverTimeMs; // reset trigger
                hoverWidget = nullptr;                     // deactivate hover widget
            }
        } else {
            // hover
            if (hoverTime && ctx.getTimeMs() > hoverTime) {
                hoverTime = 0;                             // deactivate trigger
                hoverWidget = Context::hoverWidget;        // activate hover widget if any
                hoverCursor = cursor;
                ret = true;                                // make hover appear immediately
                //DIAG(LOG("hover: %p", Input::hoverWidget));
            }
        }
        updateCalled = updateModifications = false;
        return ret;
    }

    bool Input::refreshStack(Widget* w) {
        bool modif(false);
        while (w) {
            modif |= w->input();
            w = w->getParent();
        }
        return modif;
    }

    V2f Input::cursor;
    V2f Input::cursorLeftPress;
    bool Input::updateCalled;
    bool Input::updateModifications;
    bool Input::mouseButtonAction;
    bool Input::keyboardAction;
    int Input::mouseButton;
    int Input::mouseAction;
    int Input::currentMods;
    Widget* Input::mouseButtonWidget;

    bool Input::scrollAction;
    V2f Input::scroll;

    int Input::hoverTime;
    Widget* Input::hoverWidget;
    V2f Input::hoverCursor;

}
