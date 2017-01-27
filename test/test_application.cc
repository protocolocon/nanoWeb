/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "catch.hpp"
#include "widget.h"
#include "context.h"
#include "application.h"
#include <string>

#define _ "\n"

using namespace std;
using namespace webui;

namespace {

    RequestXHR* mlApp(Application& app, const char* data) {
        return new RequestXHR(app, RequestXHR::TypeApplication, StringId(), data, strlen(data));
    }

}

TEST_CASE("basic", "[application]") {
    Context ctx;
    Application app(ctx);
    app.initializeInheritance();

    CHECK(!app.getRoot());
    CHECK(app.onLoad(mlApp(app, "Application{}")));
    REQUIRE(app.getRoot());
    CHECK(app.getRoot()->type() == Identifier::Application);
}

TEST_CASE("definition", "[application]") {
    Context ctx;
    Application app(ctx);
    app.initializeInheritance();
    auto& strMng(app.getStrMng());

    CHECK(app.onLoad(mlApp(app,
                           "Application{"
                           _"  LayoutHor {"
                           _"    width: 333           // properties before define"
                           _"    define: Definition"
                           _"    propInt16: property  // adding property to definition"
                           _"    property: 4242       // default value in definition"
                           _"  }"
                           _"  Definition {"
                           _"    property: 256        // using generic property"
                           _"  }"
                           _"  Definition {           // use default value"
                           _"    onRender: roundedRect(x, y, property, h, 1)  // use of property as generic variable"
                           _"  }"
                           _"}")));
    auto root(app.getRoot());
    REQUIRE(root);
    CHECK(root->type() == Identifier::Application);
    auto& child(root->getChildren());
    REQUIRE(child.size() == 2);
    CHECK(string(strMng.get(child[0]->type())) == "Definition");
    CHECK(string(strMng.get(child[1]->type())) == "Definition");
    CHECK(child[0]->typeWidget->get(Identifier(strMng.search("property").getId()), child[0]) == 256);
    CHECK(child[1]->typeWidget->get(Identifier(strMng.search("property").getId()), child[1]) == 4242);
    CHECK(child[0]->typeWidget->get(Identifier(strMng.search("width").getId()), child[0]) == 333);
    CHECK(child[1]->typeWidget->get(Identifier(strMng.search("width").getId()), child[1]) == 333);

}

TEST_CASE("template", "[application]") {
    Context ctx;
    Application app(ctx);
    app.initializeInheritance();

    CHECK(app.onLoad(mlApp(app,
                           "Application{"
                           _"  Template {"
                           _"    id: template"
                           _"    [ // block"
                           _"      Widget { id: a }"
                           _"      Widget { id: b }"
                           _"    ]"
                           _"    width: 100"
                           _"  }"
                           _"}")));
    auto root(app.getRoot());
    REQUIRE(root);
    CHECK(root->type() == Identifier::Application);
    auto& child(root->getChildren());
    REQUIRE(child.size() == 1);
    CHECK(child[0]->type() == Identifier::Template);
}

TEST_CASE("actions syntax", "[application]") {
    Context ctx;
    Application app(ctx);
    app.initializeInheritance();

    CHECK(app.onLoad(mlApp(app,
                           "Application{"
                           _"  onRender: ["
                           _"    strokeWidth(1.7),"
                           _"    roundedRect(-5, y+2, w-4, h-5, 10)"
                           _"  ]"
                           _"}")));
    auto root(app.getRoot());
}
