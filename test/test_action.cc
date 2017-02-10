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
    CHECK(stack[0].type == Type::Float);
    CHECK(stack[0].f == 16.5f);
}

TEST_CASE_METHOD(Fixture, "action: log", "[action]") {
    CHECK(addAction("log(\"hello\")"));
    CHECK(executeAction());
    dumpStack();
}

TEST_CASE_METHOD(Fixture, "action: assign float", "[action]") {
    CHECK(addAction("x = 42"));
    CHECK(executeAction());
    dumpStack();
    CHECK(widget.box.pos.x == 42);
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
