/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget.h"
#include "input.h"
#include "nanovg.h"
#include "context.h"
#include <cstdlib>
#include <cstddef>

using namespace std;
using namespace webui;

namespace {

    TypeWidget widgetType = {
        Identifier::Widget, sizeof(Widget), {
            { Identifier::x,              PROP(Widget, curPos.x,  Int16,        2, 0, 0) },
            { Identifier::y,              PROP(Widget, curPos.y,  Int16,        2, 0, 0) },
            { Identifier::w,              PROP(Widget, curSize.x, Int16,        2, 0, 0) },
            { Identifier::h,              PROP(Widget, curSize.y, Int16,        2, 0, 0) },
            { Identifier::id,             PROP(Widget, id,        StrId,        4, 0, 1) },
            { Identifier::width,          PROP(Widget, size[0],   SizeRelative, 2, 0, 0) },
            { Identifier::height,         PROP(Widget, size[1],   SizeRelative, 2, 0, 0) },
            { Identifier::background,     PROP(Widget, background,Color,        4, 0, 0) },
            { Identifier::foreground,     PROP(Widget, foreground,Color,        4, 0, 0) },
            { Identifier::all,            PROP(Widget, all,       Int32,        4, 0, 0) },
            { Identifier::visible,        PROP(Widget, byte0,     Bit,          1, 0, 1) },
            { Identifier::canFocus,       PROP(Widget, byte0,     Bit,          1, 3, 1) },
            { Identifier::active,         PROP(Widget, byte0,     Bit,          1, 4, 1) },
            { Identifier::draggable,      PROP(Widget, byte0,     Bit,          1, 5, 1) },
            { Identifier::onEnter,        PROP(Widget, actions,   ActionTable,  4, 0, 1) },
            { Identifier::onLeave,        PROP(Widget, actions,   ActionTable,  4, 0, 1) },
            { Identifier::onClick,        PROP(Widget, actions,   ActionTable,  4, 0, 1) },
            { Identifier::onRender,       PROP(Widget, actions,   ActionTable,  4, 0, 1) },
            { Identifier::onRenderActive, PROP(Widget, actions,   ActionTable,  4, 0, 1) },
            { Identifier::text,           PROP(Widget, text,      Text,         sizeof(void*), 0, 0) },
        }
    };

}

namespace webui {

    Widget::Widget(Widget* parent): parent(parent), text(nullptr),
                                    size { SizeRelative(100.0f, true), SizeRelative(100.0f, true) },
                                    all(0x00ff4009), actions(0) {
        typeWidget = &widgetType;
    }

    Widget::~Widget() {
        free(text);
    }

    TypeWidget& Widget::getType() {
        return widgetType;
    }

    void Widget::render(Context& ctx, int alphaMult) {
        auto& render(ctx.getRender());
        alphaMult = render.multAlpha(alphaMult, alpha);

        auto& app(ctx.getApplication());
        const auto& actionTable(app.getActionTable(actions));
        if (actionTable.onRenderActive && (active || (Input::mouseButtonWidget == this && inside)))
            app.executeNoCheck(actionTable.onRenderActive, this);
        else if (actionTable.onRender)
            app.executeNoCheck(actionTable.onRender, this);

        if (alphaMult)
            for (auto* child: children) child->render(ctx, alphaMult);
    }

    bool Widget::input(Application& app) {
        if (canFocus && Input::mouseButtonAction) {
            // this widget takes care of the event and does not propagate upwards
            Input::mouseButtonAction = false;
            if (Input::mouseButton == GLFW_MOUSE_BUTTON_LEFT) {
                if (Input::mouseAction == GLFW_PRESS)
                    Input::mouseButtonWidget = this;
                else if (Input::mouseAction == GLFW_RELEASE) {
                    // it's a click only if the widget of press is the widget of release
                    if (Input::mouseButtonWidget == this)
                        app.execute(app.getActionTable(actions).onClick, this);
                }
                return true;
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

    const Property* Widget::getProp(Identifier id) const {
        auto it(typeWidget->find(id));
        if (it == typeWidget->end()) return nullptr;
        return &it->second;
    }

    void Widget::copyFrom(const Widget* widget) {
        const auto& props = *typeWidget;
        for (const auto& prop: props)
            if (!prop.second.redundant)
                props.set(prop.first, this, props.get(prop.first, widget));
        actions = widget->actions;
        sharedActions = 1;
    }

    bool Widget::animeAlpha(Context& ctx) {
        if ( visible && alpha < 0xff) return ctx.getCloser(alpha, 0xff);
        if (!visible && alpha)        return ctx.getCloser(alpha, 0x00);
        return true;
    }

    bool Widget::update(Application& app) {
        if (!visible) return false;
        bool recurse(false), executed(false);
        if (Input::cursor >= curPos && Input::cursor < curPos + curSize) {
            // inside
            if (!inside) {
                inside = 1;
                executed = app.execute(app.getActionTable(actions).onEnter, this);
            }
            recurse = true;
        } else {
            // outside
            if (inside) {
                inside = 0;
                executed = app.execute(app.getActionTable(actions).onLeave, this);
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

    void Widget::translate(V2s t) {
        curPos += t;
        for (auto* child: children) child->translate(t);
    }

    DIAG(
        void Widget::dump(const StringManager& strMng, int level, bool props) const {
            bool recur(level >= 0);
            if (level < 0) level = 1;
            LOG("%*s%-*s: %-15s %4d %4d - %4d %4d (%6.1f%c %6.1f%c) actions: %3d  flags: %08x  size:%4d  baseType: %s  %s",
                level * 2, "", 32 - level*2, strMng.get(id),
                strMng.get(type()),
                curPos.x, curPos.y, curSize.x, curSize.y,
                size[0].dumpValue(), size[0].dumpFlags(),
                size[1].dumpValue(), size[1].dumpFlags(),
                actions, all,
                typeSize(),
                strMng.get(baseType()),
                text ? text : "");
            if (props)
                typeWidget->dump(strMng, 48);
            if (recur)
                for (const auto* child: children) child->dump(strMng, level + 1);
        });

}
