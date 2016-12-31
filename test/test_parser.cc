/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "catch_with_main.hpp"
#include "widget.h"
#include "context.h"
#include "application.h"

#define _ "\n"

using namespace webui;

namespace {

    RequestXHR* mlApp(Application& app, const char* data) {
        return new RequestXHR(app, RequestXHR::TypeApplication, StringId(), data, strlen(data));
    }

}

TEST_CASE("basic") {
    Context ctx;
    Application app(ctx);

    CHECK(!app.getRoot());
    app.onLoad(mlApp(app, "Application{}"));
    REQUIRE(app.getRoot());
    CHECK(app.getRoot()->type() == Identifier::Application);
}

TEST_CASE("definition") {
    Context ctx;
    Application app(ctx);

    app.onLoad(mlApp(app,
                      "Application{"
                     _"  LayoutHor {"
                     _"    define: Definition"
                     _"  }"
                     _"  Definition {"
                     _"  }"
                     _"}"));
    auto root(app.getRoot());
    REQUIRE(root);
    CHECK(root->type() == Identifier::Application);
    auto& child(root->getChildren());
    REQUIRE(child.size() == 1);
    CHECK(child[0]->type() == Identifier::LayoutHor);
}

TEST_CASE("template") {
    Context ctx;
    Application app(ctx);

    app.onLoad(mlApp(app,
                      "Application{"
                     _"  Template {"
                     _"    id: template"
                     _"    {"
                     _"      Widget { id: a }"
                     _"      Widget { id: b }"
                     _"    }"
                     _"    width: 100"
                     _"  }"
                     _"}"));
    auto root(app.getRoot());
    REQUIRE(root);
    CHECK(root->type() == Identifier::Application);
    auto& child(root->getChildren());
    REQUIRE(child.size() == 1);
    CHECK(child[0]->type() == Identifier::Template);
}
