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
        Fixture(): app(ctx), widget(nullptr), actions(app.getStrMng(), app.getWidgets()) {
            app.initializeInheritance();
        }

        bool addAction(const char* str) {
            REQUIRE(ml.parse(str, strlen(str)));
            ml.dumpTree();
            auto iAction(actions.add(ml, 0, ml.size(), &widget));
            actions.dump(iAction);
            return iAction;
        }

    private:
        Context ctx;
        Application app;

        Widget widget;
        MLParser ml;
        Actions actions;
    };

}

TEST_CASE_METHOD(Fixture, "action: basic", "[action]") {
    CHECK(addAction("[42, x]"));
}
