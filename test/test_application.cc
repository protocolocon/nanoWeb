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
        return new RequestXHR(Identifier::Application, StringId(), data, strlen(data));
    }

    RequestXHR* mlTemplate(const char* data, const char* id) {
        return new RequestXHR(Context::strMng.add(id), StringId(), data, strlen(data));
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
    CHECK(child[0]->typeWidget->get(Context::strMng.search("property").getId(), child[0]) == 256);
    CHECK(child[1]->typeWidget->get(Context::strMng.search("property").getId(), child[1]) == 4242);
    CHECK(child[0]->typeWidget->get(Context::strMng.search("width").getId(), child[0]) == 333);
    CHECK(child[1]->typeWidget->get(Context::strMng.search("width").getId(), child[1]) == 333);
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

TEST_CASE("application: template reload no blocks", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
                                  "Application{"
                                  _"  Template {"
                                  _"    id: template"
                                  _"    Widget {"
                                  _"      x: @"
                                  _"      Widget {"
                                  _"        w: @"
                                  _"      }"
                                  _"    }"
                                  _"    Widget {"
                                  _"      y: @"
                                  _"    }"
                                  _"  }"
                                  _"}")));
    auto root(Context::app.getRoot());
    REQUIRE(root);
    CHECK(root->type() == Identifier::Application);
    auto& child(root->getChildren());
    REQUIRE(child.size() == 1);
    CHECK(child[0]->type() == Identifier::Template);
    // load template
    CHECK(Context::app.onLoad(mlTemplate("[ -66, 1234, -77 ]", "template")));
    auto tpl(child[0]->getChildren());
    REQUIRE(tpl.size() == 2);
    auto tpl2(tpl[0]->getChildren());
    REQUIRE(tpl2.size() == 1);
    CHECK(tpl[0]->box.pos.x == -66);
    CHECK(tpl[1]->box.pos.y == -77);
    CHECK(tpl2[0]->box.size.x == 1234);
    // reload template (children are the same, as they are reused)
    CHECK(Context::app.onLoad(mlTemplate("[ -67, 4321, -76 ]", "template")));
    CHECK(tpl[0]->box.pos.x == -67);
    CHECK(tpl[1]->box.pos.y == -76);
    CHECK(tpl2[0]->box.size.x == 4321);
}

TEST_CASE("application: template reload blocks no ids", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
                                  "Application{"
                                  _"  Template {"
                                  _"    id: template"
                                  _"    ["
                                  _"      Widget {"
                                  _"        x: @"
                                  _"      }"
                                  _"    ]"
                                  _"  }"
                                  _"}")));
    auto root(Context::app.getRoot());
    REQUIRE(root);
    // load template
    CHECK(Context::app.onLoad(mlTemplate("[ [ [ 73 ], [ 84 ], [ 93 ] ] ]", "template")));
    auto tpl(root->getChildren()[0]->getChildren());
    REQUIRE(tpl.size() == 3);
    CHECK(tpl[0]->box.pos.x == 73);
    CHECK(tpl[1]->box.pos.x == 84);
    CHECK(tpl[2]->box.pos.x == 93);
    // reload with same number of children
    CHECK(Context::app.onLoad(mlTemplate("[ [ [ 8732 ], [ 9843 ], [ 1932 ] ] ]", "template")));
    REQUIRE(tpl.size() == 3); // reused widgets
    CHECK(tpl[0]->box.pos.x == 8732);
    CHECK(tpl[1]->box.pos.x == 9843);
    CHECK(tpl[2]->box.pos.x == 1932);
    // reload adding one more
    CHECK(Context::app.onLoad(mlTemplate("[ [ [ 1 ], [ 2 ], [ 3 ], [ 4 ] ] ]", "template")));
    CHECK(tpl[0]->box.pos.x == 1);
    CHECK(tpl[1]->box.pos.x == 2);
    CHECK(tpl[2]->box.pos.x == 3);
    tpl = root->getChildren()[0]->getChildren(); // need to get new children for the last one
    CHECK(tpl[3]->box.pos.x == 4);
    // reload with less widgets
    CHECK(Context::app.onLoad(mlTemplate("[ [ [ 2 ], [ 1 ] ] ]", "template")));
    tpl = root->getChildren()[0]->getChildren();
    REQUIRE(tpl.size() == 2);
    CHECK(tpl[0]->box.pos.x == 2);
    CHECK(tpl[1]->box.pos.x == 1);
}

TEST_CASE("application: template reload blocks with ids", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
                                  "Application{"
                                  _"  Template {"
                                  _"    id: template"
                                  _"    ["
                                  _"      Widget {"
                                  _"        id: @"
                                  _"        x: @"
                                  _"      }"
                                  _"    ]"
                                  _"  }"
                                  _"}")));
    auto root(Context::app.getRoot());
    REQUIRE(root);
    // load template
    CHECK(Context::app.onLoad(mlTemplate("[ [ [ id10, 10 ], [ id20, 20 ] ] ]", "template")));
    auto& tpl(root->getChildren()[0]->getChildren());
    REQUIRE(tpl.size() == 2);
    CHECK(tpl[0]->id == Context::strMng.add("id10"));
    CHECK(tpl[0]->box.pos.x == 10);
    CHECK(tpl[1]->id == Context::strMng.add("id20"));
    CHECK(tpl[1]->box.pos.x == 20);
    // mark some widgets properties
    tpl[0]->box.pos.y = 100;
    tpl[1]->box.pos.y = 200;
    // reload
    CHECK(Context::app.onLoad(mlTemplate("[ [ [ another, 99 ], [ id10, 11 ], [ id20, 22 ] ] ]", "template")));
    REQUIRE(tpl.size() == 3);
    CHECK(tpl[0]->id == Context::strMng.add("id10"));
    CHECK(tpl[0]->box.pos.x == 11);
    CHECK(tpl[0]->box.pos.y == 100);
    CHECK(tpl[1]->id == Context::strMng.add("id20"));
    CHECK(tpl[1]->box.pos.x == 22);
    CHECK(tpl[1]->box.pos.y == 200);
    CHECK(tpl[2]->id == Context::strMng.add("another"));
    CHECK(tpl[2]->box.pos.x == 99);
    // reload updating just one (others dissapear)
    CHECK(Context::app.onLoad(mlTemplate("[ [ [ id20, 24 ] ] ]", "template")));
    REQUIRE(tpl.size() == 1);
    CHECK(tpl[0]->id == Context::strMng.add("id20"));
    CHECK(tpl[0]->box.pos.x == 24);
    CHECK(tpl[0]->box.pos.y == 200);
}

TEST_CASE("application: template reload block sequences", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
                                  "Application{"
                                  _"  Template {"
                                  _"    id: template"
                                  _"    ["
                                  _"      LayoutHor {"
                                  _"        x: @"
                                  _"      }"
                                  _"    ]"
                                  _"    ["
                                  _"      LayoutVer {"
                                  _"        x: @"
                                  _"      }"
                                  _"    ]"
                                  _"  }"
                                  _"}")));
    auto root(Context::app.getRoot());
    REQUIRE(root);
    // load template
    CHECK(Context::app.onLoad(mlTemplate("[ [ [ 55 ], [ 66 ] ], [ [ 77 ], [ 88 ] ] ]", "template")));
    auto& tpl(root->getChildren()[0]->getChildren());
    REQUIRE(tpl.size() == 4);
    CHECK(tpl[0]->type() == Identifier::LayoutHor);
    CHECK(tpl[0]->box.pos.x == 55);
    CHECK(tpl[1]->type() == Identifier::LayoutHor);
    CHECK(tpl[1]->box.pos.x == 66);
    CHECK(tpl[2]->type() == Identifier::LayoutVer);
    CHECK(tpl[2]->box.pos.x == 77);
    CHECK(tpl[3]->type() == Identifier::LayoutVer);
    CHECK(tpl[3]->box.pos.x == 88);
    // reload template with different proportion of types
    CHECK(Context::app.onLoad(mlTemplate("[ [ [ 11 ] ], [ [ 22 ] ] ]", "template")));
    REQUIRE(tpl.size() == 2);
    CHECK(tpl[0]->type() == Identifier::LayoutHor);
    CHECK(tpl[0]->box.pos.x == 11);
    CHECK(tpl[1]->type() == Identifier::LayoutVer);
    CHECK(tpl[1]->box.pos.x == 22);
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
                                  _"  Widget {"
                                  _"    define: Test"
                                  _"    propText: text"
                                  _"    text: \"this is text\""
                                  _"  }"
                                  _"  Test { }"
                                  _"}")));
    auto root(Context::app.getRoot());
    REQUIRE(root);
    auto& child(root->getChildren());
    REQUIRE(child.size() == 1);
    CHECK(!strcmp((char*)child[0]->typeWidget->get(Context::strMng.search("text").getId(), child[0]), "this is text"));
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
    CHECK(child[0]->typeWidget->get(Context::strMng.search("xx").getId(), child[0]) == 42);
    CHECK(child[0]->typeWidget->get(Context::strMng.search("yy").getId(), child[0]) == 42 * 42);
}

TEST_CASE("application: double dispatch", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
                                  "Application {"
                                  _"  Widget {"
                                  _"    define: Props"
                                  _"    propId: widgetId"
                                  _"    widgetId: Props"
                                  _"    onRender: [ widgetId.x = 4242 ]"
                                  _"  }"
                                  _"  Props { }"
                                  _"}")));
    auto root(Context::app.getRoot());
    REQUIRE(root);
    auto& child(root->getChildren());
    REQUIRE(child.size() == 1);
    CHECK(string(Context::strMng.get(child[0]->type())) == "Props");
}

TEST_CASE("application: failing case", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
                                  "Application {"
                                  _"  Template {"
                                  _"    define: Tree"
                                  _"    LayoutVer {"
                                  _"    }"
                                  _"  }"
                                  _"}")));
}

TEST_CASE("application: template definition", "[application]") {
    ctx.initialize(false, false);
    CHECK(Context::app.onLoad(mlApp(
                                  "Application {"
                                  _"  Template {"
                                  _"    define: Tree"
                                  _"    ["
                                  _"      Widget {"
                                  _"        x: @"
                                  _"      }"
                                  _"    ]"
                                  _"  }"
                                  _"  Tree {"
                                  _"    id: tree"
                                  _"  }"
                                  _"  Tree {"
                                  _"    id: tree2"
                                  _"  }"
                                  _"}")));
    auto root(Context::app.getRoot());
    REQUIRE(root);
    auto& child(root->getChildren());
    REQUIRE(child.size() == 2);
    // load template
    CHECK(Context::app.onLoad(mlTemplate("[ [ [ 111 ], [ 222 ] ] ]", "tree")));
    auto& tpl(root->getChildren()[0]->getChildren());
    auto& tp2(root->getChildren()[1]->getChildren());
    REQUIRE(tpl.size() == 2);
    CHECK(tpl[0]->box.pos.x == 111);
    CHECK(tpl[1]->box.pos.x == 222);
    REQUIRE(tp2.size() == 0);
    CHECK(Context::app.onLoad(mlTemplate("[ [ [ 333 ], [ 444 ], [ 555] ] ]", "tree2")));
    REQUIRE(tp2.size() == 3);
    CHECK(tp2[0]->box.pos.x == 333);
    CHECK(tp2[1]->box.pos.x == 444);
    CHECK(tp2[2]->box.pos.x == 555);
}
