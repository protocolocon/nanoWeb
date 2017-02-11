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

    RequestXHR* mlApp(const char* data) {
        return new RequestXHR(RequestXHR::TypeApplication, StringId(), data, strlen(data));
    }

    RequestXHR* mlTemplate(const char* data, const char* id) {
        return new RequestXHR(RequestXHR::TypeTemplate, Context::strMng.add(id), data, strlen(data));
    }

}

TEST_CASE("application: basic", "[application]") {
    ctx.initialize(false, false);
    CHECK(!Context::app.getRoot());
    CHECK(Context::app.onLoad(mlApp("Application{}")));
    REQUIRE(Context::app.getRoot());
    CHECK(Context::app.getRoot()->type() == Identifier::Application);
}

TEST_CASE("application: definition", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
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
    auto root(Context::app.getRoot());
    REQUIRE(root);
    CHECK(root->type() == Identifier::Application);
    auto& child(root->getChildren());
    REQUIRE(child.size() == 2);
    CHECK(string(Context::strMng.get(child[0]->type())) == "Definition");
    CHECK(string(Context::strMng.get(child[1]->type())) == "Definition");
    CHECK(child[0]->typeWidget->get(Identifier(Context::strMng.search("property").getId()), child[0]) == 256);
    CHECK(child[1]->typeWidget->get(Identifier(Context::strMng.search("property").getId()), child[1]) == 4242);
    CHECK(child[0]->typeWidget->get(Identifier(Context::strMng.search("width").getId()), child[0]) == 333);
    CHECK(child[1]->typeWidget->get(Identifier(Context::strMng.search("width").getId()), child[1]) == 333);
}

TEST_CASE("application: template", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
                                  "Application{"
                                  _"  Template {"
                                  _"    id: template"
                                  _"    [ // block"
                                  _"      Widget {"
                                  _"        id: @"
                                  _"        x: @"
                                  _"        y: @"
                                  _"      }"
                                  _"    ]"
                                  _"    width: 100"
                                  _"  }"
                                  _"}")));
    auto root(Context::app.getRoot());
    REQUIRE(root);
    CHECK(root->type() == Identifier::Application);
    auto& child(root->getChildren());
    REQUIRE(child.size() == 1);
    CHECK(child[0]->type() == Identifier::Template);
    CHECK(child[0]->getChildren().empty());
    // load template
    CHECK(Context::app.onLoad(mlTemplate("[ [ [a, 42, 43], [b, -14, x - 1], [c, 7/2, x * 4] ] ]", "template")));
    auto& tpl(child[0]->getChildren());
    CHECK(tpl.size() == 3);
    CHECK(tpl[0]->box.pos.x == 42);
    CHECK(tpl[0]->box.pos.y == 43);
    CHECK(tpl[1]->box.pos.x == -14);
    CHECK(tpl[1]->box.pos.y == -15);
    CHECK(tpl[2]->box.pos.x == 3.5);
    CHECK(tpl[2]->box.pos.y == 14);
}

TEST_CASE("application: actions syntax", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
                                  "Application{"
                                  _"  onRender: ["
                                  _"    strokeWidth(1.7),"
                                  _"    roundedRect(-5, y+2, w-4, h-5, 10)"
                                  _"  ]"
                                  _"}")));
    auto root(Context::app.getRoot());
}

TEST_CASE("application: text", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
                                  "Application{"
                                  _"   text: \"this is text\""
                                  _"}")));
    auto root(Context::app.getRoot());
    REQUIRE(root);
    CHECK(!strcmp(root->text, "this is text"));
}

TEST_CASE("application: property computations", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
                                  "Application {"
                                  _"  Widget {"
                                  _"    define: Props"
                                  _"    propInt16: xx"
                                  _"    propInt16: yy"
                                  _"    xx: 42"
                                  _"    yy: xx * 42"
                                  _"  }"
                                  _"  Props { }"
                                  _"}")));
    auto root(Context::app.getRoot());
    REQUIRE(root);
    auto& child(root->getChildren());
    REQUIRE(child.size() == 1);
    CHECK(string(Context::strMng.get(child[0]->type())) == "Props");
    CHECK(child[0]->typeWidget->get(Identifier(Context::strMng.search("xx").getId()), child[0]) == 42);
    CHECK(child[0]->typeWidget->get(Identifier(Context::strMng.search("yy").getId()), child[0]) == 42 * 42);
}
