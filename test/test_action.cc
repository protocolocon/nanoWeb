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
        }

        bool addAction(const char* str) {
            LOG("action: '%s'", str);
            REQUIRE(ml.parse(str, strlen(str)));
            ml.dumpTree();
            iAction = Context::actions.add(ml, 0, ml.size(), &widget);
            Context::actions.dump(iAction);
            return iAction;
        }

        bool executeAction() {
            return Context::actions.execute(iAction, &widget);
        }

        const Stack& getStack() const {
            return Context::actions.getStack();
        }

        void dumpStack() const {
            Context::actions.dumpStack();
        }

    private:
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
