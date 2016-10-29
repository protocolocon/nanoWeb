/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_application.h"
#include "nanovg.h"

using namespace std;

namespace webui {

    void WidgetApplication::render(Context& ctx) {
        LOG("app");
        glClearColor(color.rf(), color.gf(), color.bf(), color.af());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        for (auto* child: children) child->render(ctx);
    }

    bool WidgetApplication::set(Identifier id, const string& value) {
        if (!Widget::set(id, value)) {
            /**/ if (id == Identifier::color) color = value;
            else return false;
        }
        return true;
    }

}
