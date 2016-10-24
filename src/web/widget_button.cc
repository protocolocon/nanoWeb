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
        LOG("button");
        auto vg(ctx.getRender().getVg());

        nvgBeginPath(vg);
        nvgRect(vg, pos[0], pos[1], size[0], size[1]);
        nvgFillColor(vg, nvgRGBA(color.r(), color.g(), color.b(), color.a()));
        nvgFill(vg);

        for (auto* child: children) child->render(ctx);
    }

    bool WidgetButton::set(const string& param, const string& value) {
        if (!Widget::set(param, value)) {
            /**/ if (param == "color") color = value;
            else return false;
        }
        return true;
    }

}
