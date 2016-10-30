/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_application.h"
#include "nanovg.h"
#include "application.h"
#include <cassert>

using namespace std;

namespace webui {

    void WidgetApplication::render(Context& ctx, int alphaMult) {
        assert(visible);
        glClearColor(color.rf(), color.gf(), color.bf(), color.af());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        for (auto* child: children) child->render(ctx, alphaMult);
    }

    bool WidgetApplication::set(Application& app, Identifier id, int iEntry, int fEntry) {
        if (!Widget::set(app, id, iEntry, fEntry)) {
            switch (id) {
            case Identifier::color:
                if (fEntry > iEntry + 1) { LOG("color expects a simple value"); return false; }
                color = app.entry(iEntry).pos;
                return true;
            default:
                return false;
            }
        }
        return true;
    }

}
