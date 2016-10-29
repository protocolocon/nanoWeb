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
        if (visible) {
            LOG("widget");
            for (auto* child: children) child->render(ctx);
        }
    }

    bool Widget::layout(V2s posAvail, V2s sizeAvail, float time) {
        bool stable(true);
        if (visible) {
            curPos = posAvail;
            curSize = sizeAvail;

            for (auto* child: children) stable &= child->layout(curPos, child->getSizeTarget(curSize), time);
        }
        return stable;
    }

    bool Widget::set(Application& app, Identifier id, int iEntry) {
        pair<const char*, int> ss;
        switch (id) {
        case Identifier::id:
            this->id = app.entryAsStrId(iEntry);
            return true;
        case Identifier::width:
            ss = app.entryAsStrSize(iEntry);
            if ((wRelative = ss.first[ss.second - 1] == '%')) size.x = int(atof(ss.first) * 2.56f + 0.5f); else size.x = atoi(ss.first);
            return true;
        case Identifier::height:
            ss = app.entryAsStrSize(iEntry);
            if ((hRelative = ss.first[ss.second - 1] == '%')) size.y = int(atof(ss.first) * 2.56f + 0.5f); else size.y = atoi(ss.first);
            return true;
        case Identifier::onEnter:
        case Identifier::onLeave:
            return app.addAction(id, iEntry, actions);
        default:
            return false;
        }
    }

    bool Widget::update(Application& app, V2s cursor) {
        if (!visible) return false;
        bool recurse(false), executed(false);
        if (cursor >= curPos && cursor < curPos + curSize) {
            // inside
            if (!inside) {
                inside = 1;
                executed = app.execute(app.getActionTable(actions).onEnter);
            }
            recurse = true;
        } else {
            // outside
            if (inside) {
                inside = 0;
                executed = app.execute(app.getActionTable(actions).onLeave);
                recurse = true;
            }
        }
        if (recurse)
            for (auto* child: children)
                executed |= child->update(app, cursor);
        return executed;
    }

    void Widget::dump(const StringManager& strMng, int level) const {
        LOG("%*s%s: %4d %4d - %4d %4d (%4d%c %4d%c) actions: %d", level * 2, "", strMng.get(id),
            curPos.x, curPos.y, curSize.x, curSize.y, size.x, wRelative ? '%' : ' ', size.y, hRelative ? '%' : ' ', actions);
        for (const auto* child: children) child->dump(strMng, level + 1);
    }

}
