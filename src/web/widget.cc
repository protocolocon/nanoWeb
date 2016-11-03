/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget.h"
#include "main.h"
#include "input.h"
#include "nanovg.h"
#include <cstdlib>

using namespace std;

namespace webui {

    void Widget::render(Context& ctx, int alphaMult) {
        auto& render(ctx.getRender());
        alphaMult = render.multAlpha(alphaMult, alpha);

        auto& app(ctx.getApplication());
        const auto& actionTable(app.getActionTable(actions));
        if (actionTable.onRender) {
            auto& renderCtx(app.getActionRenderContext());
            renderCtx.pos = curPos;
            renderCtx.size = curSize;
            app.executeNoCheck(actionTable.onRender);
        }

        if (alphaMult)
            for (auto* child: children) child->render(ctx, alphaMult);
    }

    bool Widget::input(Application& app) {
        if (Input::mouseButtonAction) {
            // this widget takes care of the event and does not propagate upwards
            Input::mouseButtonAction = false;
            if (Input::mouseButton == GLFW_MOUSE_BUTTON_LEFT) {
                if (Input::mouseAction == GLFW_PRESS)
                    Input::mouseButtonPress = this;
                else if (Input::mouseAction == GLFW_RELEASE) {
                    // it's a click only if the widget of press is the widget of release
                    if (Input::mouseButtonPress == this)
                        return app.execute(app.getActionTable(actions).onClick);
                }
            }
        }
        return false;
    }

    bool Widget::layout(Context& ctx, V2s posAvail, V2s sizeAvail) {
        bool stable(true);
        curPos = posAvail;
        curSize = sizeAvail;
        if (visible)
            for (auto* child: children) stable &= child->layout(ctx, curPos, child->getSizeTarget(curSize));
        return animeAlpha(ctx) && stable;
    }

    bool Widget::animeAlpha(Context& ctx) {
        if ( visible && alpha < 0xff) return ctx.getCloser(alpha, 0xff);
        if (!visible && alpha)        return ctx.getCloser(alpha, 0x00);
        return true;
    }

    bool Widget::set(Application& app, Identifier id, int iEntry, int fEntry) {
        pair<const char*, int> ss;
        switch (id) {
        case Identifier::id:
            if (fEntry > iEntry + 1) { LOG("id expects a simple value"); return false; }
            this->id = app.entryAsStrId(iEntry);
            return true;
        case Identifier::width:
            if (fEntry > iEntry + 1) { LOG("width expects a simple value"); return false; }
            ss = app.entryAsStrSize(iEntry);
            if ((wRelative = ss.first[ss.second - 1] == '%')) size.x = int(atof(ss.first) * 2.56f + 0.5f); else size.x = atoi(ss.first);
            return true;
        case Identifier::height:
            if (fEntry > iEntry + 1) { LOG("height expects a simple value"); return false; }
            ss = app.entryAsStrSize(iEntry);
            if ((hRelative = ss.first[ss.second - 1] == '%')) size.y = int(atof(ss.first) * 2.56f + 0.5f); else size.y = atoi(ss.first);
            return true;
        case Identifier::onEnter:
        case Identifier::onLeave:
        case Identifier::onClick:
        case Identifier::onRender:
            return app.addAction(id, iEntry, fEntry, actions);
        default:
            return false;
        }
    }

    bool Widget::update(Application& app) {
        if (!visible) return false;
        bool recurse(false), executed(false);
        if (Input::cursor >= curPos && Input::cursor < curPos + curSize) {
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
                executed |= child->update(app);

        // specific input actions
        if (inside && (Input::mouseButtonAction || Input::keyboardAction)) executed |= input(app);

        return executed;
    }

    void Widget::dump(const StringManager& strMng, int level) const {
        LOG("%*s%s: %4d %4d - %4d %4d (%4d%c %4d%c) actions: %d", level * 2, "", strMng.get(id),
            curPos.x, curPos.y, curSize.x, curSize.y, size.x, wRelative ? '%' : ' ', size.y, hRelative ? '%' : ' ', actions);
        for (const auto* child: children) child->dump(strMng, level + 1);
    }

}
