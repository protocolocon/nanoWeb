/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "catch.hpp"
#include "ml_parser.h"

#define _ "\n"

#define DUMP(x, ...)       x, ##__VA_ARGS__

using namespace std;
using namespace webui;

namespace {

    bool stringCompare(const char* str, const char* pos, int size) {
        return int(strlen(str)) == size && !strncmp(str, pos, size);
    }

}

TEST_CASE("parser: id", "[parser]") {
    MLParser ml;
    const char* str("s0mething");
    REQUIRE(ml.parse(str, strlen(str)));
    DUMP(ml.dumpTree());
    REQUIRE(ml.size() == 1);
    CHECK(ml[0].type() == MLParser::EntryType::Id);
    CHECK(stringCompare("s0mething", ml[0].pos, ml.size(0)));
}

TEST_CASE("parser: number", "[parser]") {
    MLParser ml;
    const char* str("123.4");
    REQUIRE(ml.parse(str, strlen(str)));
    DUMP(ml.dumpTree());
    REQUIRE(ml.size() == 1);
    CHECK(ml[0].type() == MLParser::EntryType::Number);
    CHECK(stringCompare("123.4", ml[0].pos, ml.size(0)));
}

TEST_CASE("parser: number negative", "[parser]") {
    MLParser ml;
    const char* str("-123.4");
    REQUIRE(ml.parse(str, strlen(str)));
    DUMP(ml.dumpTree());
    REQUIRE(ml.size() == 1);
    CHECK(ml[0].type() == MLParser::EntryType::Number);
    CHECK(stringCompare("-123.4", ml[0].pos, ml.size(0)));
}

TEST_CASE("parser: number invalid", "[parser]") {
    MLParser ml;
    const char* str("-12.3.4");
    CHECK(!ml.parse(str, strlen(str)));
    str = "12f";
    CHECK(!ml.parse(str, strlen(str)));
}

TEST_CASE("parser: function", "[parser]") {
    MLParser ml;
    const char* str("combine(id1, id2, id3)");
    REQUIRE(ml.parse(str, strlen(str)));
    DUMP(ml.dumpTree());
    REQUIRE(ml.size() == 4);
    CHECK(ml[0].type() == MLParser::EntryType::Function);
    CHECK(ml[1].type() == MLParser::EntryType::Id);
    CHECK(ml[2].type() == MLParser::EntryType::Id);
    CHECK(ml[3].type() == MLParser::EntryType::Id);
    CHECK(ml[0].next == ml.size());
    CHECK(stringCompare("combine", ml[0].pos, ml.size(0)));
}

TEST_CASE("parser: function empty", "[parser]") {
    MLParser ml;
    const char* str("combine()");
    REQUIRE(ml.parse(str, strlen(str)));
    DUMP(ml.dumpTree());
    REQUIRE(ml.size() == 1);
    CHECK(ml[0].type() == MLParser::EntryType::Function);
    CHECK(ml[0].next == ml.size());
    CHECK(stringCompare("combine", ml[0].pos, ml.size(0)));
}

TEST_CASE("parser: color", "[parser]") {
    MLParser ml;
    const char* str("#1234abcd");
    REQUIRE(ml.parse(str, strlen(str)));
    DUMP(ml.dumpTree());
    REQUIRE(ml.size() == 1);
    CHECK(ml[0].type() == MLParser::EntryType::Color);
    CHECK(stringCompare("#1234abcd", ml[0].pos, ml.size(0)));
}

TEST_CASE("parser: string", "[parser]") {
    MLParser ml;
    const char* str("\"str\"");
    REQUIRE(ml.parse(str, strlen(str)));
    DUMP(ml.dumpTree());
    REQUIRE(ml.size() == 1);
    CHECK(ml[0].type() == MLParser::EntryType::String);
    CHECK(stringCompare("\"str\"", ml[0].pos, ml.size(0)));
}

TEST_CASE("parser: list", "[parser]") {
    MLParser ml;
    const char* str("[#ffaacc, someId, [id5, id6], anotherId]");
    REQUIRE(ml.parse(str, strlen(str)));
    DUMP(ml.dumpTree());
    REQUIRE(ml.size() == 7);
    CHECK(ml[0].type() == MLParser::EntryType::List);
    CHECK(ml[3].type() == MLParser::EntryType::List);
    CHECK(stringCompare("#ffaacc", ml[1].pos, ml.size(1)));
    CHECK(stringCompare("someId", ml[2].pos, ml.size(2)));
    CHECK(stringCompare("id5", ml[4].pos, ml.size(4)));
    CHECK(stringCompare("id6", ml[5].pos, ml.size(5)));
    CHECK(stringCompare("anotherId", ml[6].pos, ml.size(6)));
}

TEST_CASE("parser: operation", "[parser]") {
    MLParser ml;
    const char* str("a + b * (c + f(d))");
    REQUIRE(ml.parse(str, strlen(str)));
    DUMP(ml.dumpTree());
    REQUIRE(ml.size() == 8);
    CHECK(ml[1].type() == MLParser::EntryType::Operator);
}

TEST_CASE("parser: operation invalid", "[parser]") {
    MLParser ml;
    const char* str("a + + b");
    CHECK(!ml.parse(str, strlen(str)));
}

TEST_CASE("parser: operation invalid 2", "[parser]") {
    MLParser ml;
    const char* str("* c");
    CHECK(!ml.parse(str, strlen(str)));
}

TEST_CASE("parser: object", "[parser]") {
    MLParser ml;
    const char* str("Object {         // comment"
                   _"  key: value     // comment"
                   _"  id: [list]     // comment"
                   _"  obj { }        // comment"
                   _"  obj2 {         // comment"
                   _"    obj3 {       // comment"
                   _"      [          // block"
                   _"        id: f(x) // comment"
                   _"      ]          // comment"
                   _"    }            // comment"
                   _"  }              // comment"
                   _"}                // comment");
    REQUIRE(ml.parse(str, strlen(str)));
    DUMP(ml.dumpTree());
    CHECK(ml[0].type() == MLParser::EntryType::Object);
    CHECK(ml[0].next == ml.size());
}

TEST_CASE("parser: wildcar", "[parser]") {
    MLParser ml;
    const char* str("@");
    CHECK(ml.parse(str, strlen(str)));
    DUMP(ml.dumpTree());
    REQUIRE(ml.size() == 1);
    CHECK(ml[0].type() == MLParser::EntryType::Wildcar);
    CHECK(ml[0].next == ml.size());
}
