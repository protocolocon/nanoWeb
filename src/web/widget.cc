/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget.h"
#include "main.h"
#include "input.h"
#include "nanovg.h"
#include "properties.h"
#include <cstdlib>
#include <cstddef>

using namespace std;
using namespace webui;

namespace {

    Properties widgetProperties = {
        { Identifier::x,         PROP(Widget, curPos.x,  Int16,        2, 0, 0) },
        { Identifier::y,         PROP(Widget, curPos.y,  Int16,        2, 0, 0) },
        { Identifier::w,         PROP(Widget, curSize.x, Int16,        2, 0, 0) },
        { Identifier::h,         PROP(Widget, curSize.y, Int16,        2, 0, 0) },
        { Identifier::id,        PROP(Widget, id,        StrId,        4, 0, 0) },
        { Identifier::width,     PROP(Widget, size[0],   SizeRelative, 2, 0, 0) },
        { Identifier::height,    PROP(Widget, size[1],   SizeRelative, 2, 0, 0) },
        { Identifier::background,PROP(Widget, background,Color,        4, 0, 0) },
        { Identifier::foreground,PROP(Widget, foreground,Color,        4, 0, 0) },
        { Identifier::all,       PROP(Widget, all,       Int32,        4, 0, 0) },
        { Identifier::onEnter,   PROP(Widget, actions,   ActionTable,  4, 0, 0) },
        { Identifier::onLeave,   PROP(Widget, actions,   ActionTable,  4, 0, 0) },
        { Identifier::onClick,   PROP(Widget, actions,   ActionTable,  4, 0, 0) },
        { Identifier::onRender,  PROP(Widget, actions,   ActionTable,  4, 0, 0) },
    };

}

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

    const Properties& Widget::getProps() const {
        return widgetProperties;
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
            curPos.x, curPos.y, curSize.x, curSize.y, size[0].size, size[0].relative ? '%' : ' ', size[1].size, size[1].relative ? '%' : ' ', actions);
        for (const auto* child: children) child->dump(strMng, level + 1);
    }

}
