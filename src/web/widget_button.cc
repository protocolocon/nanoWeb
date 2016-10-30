/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_button.h"
#include "main.h"
#include "nanovg.h"

using namespace std;

namespace webui {

    void WidgetButton::render(Context& ctx) {
        if (visible) {
            auto vg(ctx.getRender().getVg());

            nvgBeginPath(vg);
            nvgRect(vg, curPos[0], curPos[1], curSize[0], curSize[1]);
            nvgFillColor(vg, nvgRGBA(color.r(), color.g(), color.b(), color.a()));
            nvgFill(vg);

            for (auto* child: children) child->render(ctx);
        }
    }

    bool WidgetButton::set(Application& app, Identifier id, int iEntry, int fEntry) {
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
