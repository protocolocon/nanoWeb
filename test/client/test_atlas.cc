/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "catch.hpp"
#include "atlas.h"

using namespace std;
using namespace render;

TEST_CASE("basic", "[atlas]") {
    Atlas atlas;
    CHECK(atlas.add(10, 10) == 0);
    CHECK(atlas.get(0) == Atlas::Sprite({ 0, 0, 10, 10 }));
    CHECK(atlas.add(10, 10) == 1);
    CHECK(atlas.get(1) == Atlas::Sprite({ 11, 0, 21, 10 }));
    atlas.remove(0);
    atlas.remove(1);
    CHECK(atlas.check());
    //atlas.dump();
}

TEST_CASE("stress", "[atlas]") {
    Atlas atlas;
    vector<int> sprites;
    for (int i = 0; i < 3000; i++) {
        sprites.push_back(atlas.add(rand() % 40, rand() % 40));
        CHECK(sprites.back() != -1);
        if (!(i & 3)) {
            atlas.remove(sprites.back());
            sprites.pop_back();
        }
    }
    CHECK(atlas.check());
    //atlas.dump();
    for (auto s: sprites) atlas.remove(s);
    CHECK(atlas.check());
}
