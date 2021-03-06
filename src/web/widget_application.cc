/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_application.h"
#include "input.h"
#include "nanovg.h"
#include "context.h"
#include "application.h"
#include <cassert>

using namespace std;
using namespace webui;

namespace {

    TypeWidget widgetApplicationType = {
        Identifier::Application, sizeof(WidgetApplication), {
            { Identifier::background,     PROP(WidgetApplication, background, Color,        4, 0, 0) },
            { Identifier::mouseX,         PROP(WidgetApplication, cursor.x,   Float,        4, 0, 0) },
            { Identifier::mouseY,         PROP(WidgetApplication, cursor.y,   Float,        4, 0, 0) },
            { Identifier::hoverX,         PROP(WidgetApplication, hover.x,    Float,        4, 0, 0) },
            { Identifier::hoverY,         PROP(WidgetApplication, hover.y,    Float,        4, 0, 0) },
        }
    };

}

namespace webui {

    WidgetApplication::WidgetApplication(Widget* parent): Widget(parent) {
        typeWidget = &widgetApplicationType;
    }

    TypeWidget& WidgetApplication::getType() {
        return widgetApplicationType;
    }

    void WidgetApplication::render(int alphaMult) {
        assert(visible);
        glClearColor(background.rf(), background.gf(), background.bf(), background.af());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        const auto& actionTable(Context::app.getActionTable(actions));
        Context::actions.execute(actionTable.onRender, this);

        // visibility of layouts
        Context::renderVisibilityBox.pos.assign(0, 0);
        Context::renderVisibilityBox.size.assign(Context::render.getWidth(), Context::render.getHeight());

        // mouse cursor
        cursor = Input::cursor;
        hover = Input::hoverCursor;

        for (auto* child: children) child->render(alphaMult);
    }

}
