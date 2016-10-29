/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget.h"
#include "main.h"
#include "nanovg.h"
#include <cstdlib>

using namespace std;

namespace webui {

    void Widget::render(Context& ctx) {
        LOG("widget");
        for (auto* child: children) child->render(ctx);
    }

    bool Widget::layout(V2s posAvail, V2s sizeAvail, float time) {
        curPos = posAvail;
        curSize = sizeAvail;

        bool stable(true);
        for (auto* child: children) stable &= child->layout(curPos, child->getSizeTarget(curSize), time);
        return stable;
    }

    bool Widget::set(Identifier id, const string& value) {
        if (id == Identifier::id) this->id = value;
        else if (id == Identifier::width) {
            if ((wRelative = value[value.size() - 1] == '%')) size.x = int(atof(value.c_str()) * 2.56f + 0.5f); else size.x = atoi(value.c_str());
        } else if (id == Identifier::height) {
            if ((hRelative = value[value.size() - 1] == '%')) size.y = int(atof(value.c_str()) * 2.56f + 0.5f); else size.y = atoi(value.c_str());
        } else return false;
        return true;
    }

    void Widget::dump(int level) const {
        LOG("%*s%s: %4d %4d - %4d %4d (%4d%c %4d%c)", level * 2, "", id.c_str(),
            curPos.x, curPos.y, curSize.x, curSize.y, size.x, wRelative ? '%' : ' ', size.y, hRelative ? '%' : ' ');
        for (const auto* child: children) child->dump(level + 1);
    }

}
