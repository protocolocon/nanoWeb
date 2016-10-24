/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget.h"
#include "main.h"
#include "nanovg.h"

using namespace std;

namespace webui {

    void Widget::render(Context& ctx) {
        LOG("widget");
        for (auto* child: children) child->render(ctx);
    }

    bool Widget::set(const string& param, const string& value) {
        /**/ if (param == "id") id = value;
        else if (param == "width") size[0] = atoi(value.c_str());
        else if (param == "height") size[1] = atoi(value.c_str());
        else return false;
        return true;
    }

    void Widget::dump(int level) const {
        LOG("%*s%s: -", level * 2, "", id.c_str());
        for (const auto* child: children) child->dump(level + 1);
    }

}
