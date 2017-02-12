/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "catch.hpp"
#include "context.h"
#include "action.h"
#include "widget.h"
#include "ml_parser.h"
#include "application.h"

using namespace std;
using namespace webui;

namespace {

    class Fixture {
    public:
        Fixture(): widget(nullptr) {
            ctx.initialize(false, false);
            auto& ws(Context::app.getWidgets());
            ws[Context::strMng.add("alpha")] = Context::app.createWidget(Identifier::Widget, nullptr);
            ws[Context::strMng.add("omega")] = Context::app.createWidget(Identifier::Widget, nullptr);
        }

        bool addAction(const char* str) {
            LOG("action: '%s'", str);
            REQUIRE(ml.parse(str, strlen(str)));
            ml.dumpTree();
            iAction = Context::actions.add(ml, 0, ml.size());
            Context::actions.dump(iAction);
            return iAction;
        }

        bool executeAction() {
            CHECK(Context::actions.execute<true>(iAction, &widget));
            return Context::actions.execute(iAction, &widget);
        }

        const Stack& getStack() const {
            return Context::actions.getStack();
        }

        void dumpStack() const {
            Context::actions.dumpStack();
        }

    protected:
        Widget widget;
        MLParser ml;
        int iAction;
    };

}

TEST_CASE_METHOD(Fixture, "action: formula", "[action]") {
    CHECK(addAction("(12 + 21) / 2"));
    CHECK(executeAction());
    dumpStack();
    const auto& stack(getStack());
    CHECK(stack.size() == 1);
    CHECK(stack[0].f == 16.5f);
}

TEST_CASE_METHOD(Fixture, "action: formula II", "[action]") {
    CHECK(addAction("(((2 * 3) + 5 + (4 * 5) + 9) / 10) + 1.234"));
    CHECK(executeAction());
    dumpStack();
    const auto& stack(getStack());
    CHECK(stack.size() == 1);
    CHECK(stack[0].f == 5.234f);
}

TEST_CASE_METHOD(Fixture, "action: log", "[action]") {
    CHECK(addAction("log(\"hello\")"));
    CHECK(executeAction());
    dumpStack();
}

TEST_CASE_METHOD(Fixture, "action: font", "[action]") {
    CHECK(addAction("font(\"font.ttf\", 40)"));
    CHECK(Context::actions.execute<true>(iAction, &widget));
    dumpStack();
}

TEST_CASE_METHOD(Fixture, "action: assign float", "[action]") {
    CHECK(addAction("x = 42"));
    CHECK(executeAction());
    dumpStack();
    CHECK(widget.box.pos.x == 42);
}

TEST_CASE_METHOD(Fixture, "action: assign text", "[action]") {
    CHECK(addAction("text = \"hello\""));
    CHECK(executeAction());
    dumpStack();
    CHECK(!strcmp(widget.text, "hello"));
}

TEST_CASE_METHOD(Fixture, "action: assign id", "[action]") {
    CHECK(addAction("id = SomeString"));
    CHECK(executeAction());
    dumpStack();
    CHECK(widget.id == Context::strMng.search("SomeString"));
}

TEST_CASE_METHOD(Fixture, "action: assign var float", "[action]") {
    widget.box.pos.y = 43;
    CHECK(addAction("x = y"));
    CHECK(executeAction());
    dumpStack();
    CHECK(widget.box.pos.x == 43);
}

TEST_CASE_METHOD(Fixture, "action: assign color", "[action]") {
    CHECK(addAction("background = #00112233"));
    CHECK(executeAction());
    dumpStack();
    CHECK(widget.background.rgba() == 0x00112233);
}

TEST_CASE_METHOD(Fixture, "action: assign bit", "[action]") {
    CHECK(addAction("[visible = 1, canFocus = 0]"));
    CHECK(executeAction());
    dumpStack();
    CHECK(widget.visible == 1);
    CHECK(widget.canFocus == 0);
    CHECK(addAction("[visible = 0, canFocus = 1]"));
    CHECK(executeAction());
    dumpStack();
    CHECK(widget.visible == 0);
    CHECK(widget.canFocus == 1);
}

TEST_CASE_METHOD(Fixture, "action: assign foreign float", "[action]") {
    auto& ws(Context::app.getWidgets());
    ws[Context::strMng.search("omega")]->box.pos.y = 144.0f;
    ws[Context::strMng.search("alpha")]->box.pos.y = -35.2f;
    CHECK(addAction("[alpha.x = omega.y, omega.x = alpha.y]"));
    CHECK(executeAction());
    dumpStack();
    CHECK(ws[Context::strMng.search("alpha")]->box.pos.x == 144.0f);
    CHECK(ws[Context::strMng.search("omega")]->box.pos.x == -35.2f);
}

TEST_CASE_METHOD(Fixture, "action: assign float promotion from bit", "[action]") {
    CHECK(addAction("[visible = 1, x = visible]"));
    CHECK(executeAction());
    dumpStack();
    CHECK(widget.box.pos.x == 1);
}

TEST_CASE_METHOD(Fixture, "action: assign float promotion from foreign bit", "[action]") {
    auto& ws(Context::app.getWidgets());
    ws[Context::strMng.search("alpha")]->visible = 1;
    ws[Context::strMng.search("omega")]->visible = 0;
    CHECK(addAction("[x = alpha.visible, y = omega.visible]"));
    CHECK(executeAction());
    dumpStack();
    CHECK(widget.box.pos.x == 1);
    CHECK(widget.box.pos.y == 0);
}

TEST_CASE_METHOD(Fixture, "action: change foreign bit", "[action]") {
    auto& ws(Context::app.getWidgets());
    ws[Context::strMng.search("alpha")]->visible = 1;
    CHECK(addAction("alpha.visible = 1 - alpha.visible"));
    CHECK(executeAction());
    CHECK(ws[Context::strMng.search("alpha")]->visible == 0);
    CHECK(addAction("alpha.visible = 1 - alpha.visible"));
    CHECK(executeAction());
    CHECK(ws[Context::strMng.search("alpha")]->visible == 1);
}

TEST_CASE_METHOD(Fixture, "action: assign color with double dispatch", "[action]") {
    widget.id = Context::strMng.add("alpha");
    CHECK(addAction("id.background = #00112233"));
    CHECK(executeAction());
    auto& ws(Context::app.getWidgets());
    CHECK(ws[widget.id]->background.rgba() == 0x00112233);
}

TEST_CASE_METHOD(Fixture, "action: assign id from foreign", "[action]") {
    auto id(Context::strMng.add("someId"));
    auto& ws(Context::app.getWidgets());
    ws[Context::strMng.search("alpha")]->id = id;
    CHECK(addAction("id = alpha.id"));
    CHECK(executeAction());
    CHECK(widget.id == id);
}

TEST_CASE_METHOD(Fixture, "action: assign id to foreign", "[action]") {
    auto id(Context::strMng.add("anotherRandomId"));
    widget.id = id;
    CHECK(addAction("alpha.id = id"));
    CHECK(executeAction());
    auto& ws(Context::app.getWidgets());
    CHECK(ws[Context::strMng.search("alpha")]->id == id);
}

TEST_CASE_METHOD(Fixture, "action: assign id to and from foreign", "[action]") {
    auto id(Context::strMng.add("anotherRandomIdMore"));
    auto& ws(Context::app.getWidgets());
    ws[Context::strMng.search("alpha")]->id = id;
    CHECK(addAction("omega.id=alpha.id"));
    CHECK(executeAction());
    CHECK(ws[Context::strMng.search("omega")]->id == id);
}

TEST_CASE_METHOD(Fixture, "action: triple dispatch", "[action]") {
    // in alpha we store the reference of the widget (omega) to change a parameter
    auto& ws(Context::app.getWidgets());
    widget.id = Context::strMng.search("alpha");
    LOG("widget %p   alpha %p   omefa %p", &widget, ws[Context::strMng.search("alpha")], ws[Context::strMng.search("omega")]);
    ws[Context::strMng.search("alpha")]->id = Context::strMng.add("omega");
    CHECK(addAction("[id = alpha.id, id.x = 4490]"));
    CHECK(executeAction());
    CHECK(ws[Context::strMng.search("omega")]->box.pos.x == 4490);
}
