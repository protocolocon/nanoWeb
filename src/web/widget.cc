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
            { Identifier::x,              PROP(Widget, box.pos.x,  Float,        4, 0, 0) },
            { Identifier::y,              PROP(Widget, box.pos.y,  Float,        4, 0, 0) },
            { Identifier::w,              PROP(Widget, box.size.x, Float,        4, 0, 0) },
            { Identifier::h,              PROP(Widget, box.size.y, Float,        4, 0, 0) },
            { Identifier::id,             PROP(Widget, id,         Id,           4, 0, 1) },
            { Identifier::width,          PROP(Widget, size[0],    SizeRelative, 2, 0, 0) },
            { Identifier::height,         PROP(Widget, size[1],    SizeRelative, 2, 0, 0) },
            { Identifier::all,            PROP(Widget, all,        Int32,        4, 0, 0) },
            { Identifier::visible,        PROP(Widget, byte0,      Bit,          1, 0, 1) },
            { Identifier::canFocus,       PROP(Widget, byte0,      Bit,          1, 3, 1) },
            { Identifier::active,         PROP(Widget, byte0,      Bit,          1, 4, 1) },
            { Identifier::draggable,      PROP(Widget, byte0,      Bit,          1, 5, 1) },
            { Identifier::onEnter,        PROP(Widget, actions,    ActionTable,  4, 0, 1) },
            { Identifier::onLeave,        PROP(Widget, actions,    ActionTable,  4, 0, 1) },
            { Identifier::onClick,        PROP(Widget, actions,    ActionTable,  4, 0, 1) },
            { Identifier::onRender,       PROP(Widget, actions,    ActionTable,  4, 0, 1) },
            { Identifier::onRenderActive, PROP(Widget, actions,    ActionTable,  4, 0, 1) },
        }
    };

}

namespace webui {

    Widget::Widget(Widget* parent): size { SizeRelative(100.0f, true), SizeRelative(100.0f, true) }, parent(parent),
                                    all(0x00ff4009), actions(0) {
        typeWidget = &widgetType;
    }

    Widget::~Widget() {
        // free text properties
        for (auto& prop: *typeWidget)
            if (prop.second.type == Type::Text)
                free(reinterpret_cast<char**>(this)[prop.second.pos]);
    }

    TypeWidget& Widget::getType() {
        return widgetType;
    }

    void Widget::render(int alphaMult) {
        alphaMult = Context::render.multAlpha(alphaMult, alpha);

        const auto& actionTable(Context::app.getActionTable(actions));
        if (actionTable.onRenderActive && (active || (Input::mouseButtonWidget == this && inside)))
            Context::actions.execute(actionTable.onRenderActive, this);
        else
            Context::actions.execute(actionTable.onRender, this);

        if (alphaMult)
            for (auto* child: children) child->render(alphaMult);
    }

    bool Widget::input() {
        if (canFocus && Input::mouseButtonAction) {
            // this widget takes care of the event and does not propagate upwards
            Input::mouseButtonAction = false;
            if (Input::mouseButton == GLFW_MOUSE_BUTTON_LEFT) {
                if (Input::mouseAction == GLFW_PRESS)
                    Input::mouseButtonWidget = this;
                else if (Input::mouseAction == GLFW_RELEASE) {
                    // it's a click only if the widget of press is the widget of release
                    if (Input::mouseButtonWidget == this)
                        Context::actions.execute(Context::app.getActionTable(actions).onClick, this);
                }
                return true;
            }
        }
        return false;
    }

    bool Widget::layout(const Box4f& boxAvail) {
        bool stable(true);
        box = boxAvail;
        if (visible)
            for (auto* child: children) stable &= child->layout(box); //curPos, child->getSizeTarget(curSize));
        return animeAlpha() && stable;
    }

    bool Widget::setData() {
        DIAG(LOG("widget does not implement setData()"));
        return false;
    }

    const Property* Widget::getProp(StringId id) const {
        auto it(typeWidget->find(Identifier(id.getId())));
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

    bool Widget::animeAlpha() {
        if ( visible && alpha < 0xff) return ctx.getCloser(alpha, 0xff);
        if (!visible && alpha)        return ctx.getCloser(alpha, 0x00);
        return true;
    }

    bool Widget::update() {
        if (!visible) return false;
        bool recurse(false), executed(false);
        if (Input::cursor >= box.pos && Input::cursor < box.pos + box.size) {
            // inside
            if (!inside) {
                inside = 1;
                executed = Context::actions.execute(Context::app.getActionTable(actions).onEnter, this);
            }
            recurse = true;
        } else {
            // outside
            if (inside) {
                inside = 0;
                executed = Context::actions.execute(Context::app.getActionTable(actions).onLeave, this);
                recurse = true;
            }
        }
        if (recurse)
            for (auto* child: children)
                executed |= child->update();

        // specific input actions
        if (inside && (Input::mouseButtonAction || Input::keyboardAction)) executed |= input();

        return executed;
    }

    void Widget::translate(V2f t) {
        box.pos += t;
        for (auto* child: children) child->translate(t);
    }

    DIAG(
        void Widget::dump(int level, bool props) const {
            bool recur(level >= 0);
            if (level < 0) level = 1;
            LOG("%*s%-*s: %-15s %7.1f %7.1f - %7.1f %7.1f (%6.1f%c %6.1f%c) actions: %3d  flags: %08x  size:%4d  baseType: %s",
                level * 2, "", 32 - level*2, Context::strMng.get(id),
                Context::strMng.get(type()),
                box.pos.x, box.pos.y, box.size.x, box.size.y,
                size[0].dumpValue(), size[0].dumpFlags(),
                size[1].dumpValue(), size[1].dumpFlags(),
                actions, all,
                typeSize(),
                Context::strMng.get(baseType()));
            if (props)
                typeWidget->dump(48, this);
            if (recur)
                for (const auto* child: children) child->dump(level + 1);
        });

}
