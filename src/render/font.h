/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "font_ll.h"
#include "compatibility.h"
#include <unordered_map>
#include <unordered_set>

namespace render {

    class Atlas;
    class VertexBuffer;

    class Font: public FontLL {
    public:
        Font();
        bool initializationInProgress();
        void markInitInProgress();

        void text(const char* text, const char* textEnd, int height, Atlas& atlas, VertexBuffer& vertex);

        // debug, populate all font codepoints in atlas
        void populate(int height, Atlas& atlas);

    private:
        std::unordered_map<uint32_t, int> codepointIndex; // caches codepoint to glyph index map
        struct Glyph {
            Glyph() { }
            Glyph(int index, int height, int atlas = -1, int advance = 0, int x0 = 0, int y0 = 0):
                index(index), atlas(atlas), height(height), advance(advance), x0(x0), y0(y0) { }

            int index;       // index in the font
            int atlas;       // index in atlas
            uint16_t height; // height of the font
            int16_t advance, x0, y0;
            size_t operator()(const Glyph& g) const { return g.index + g.height * 61; }
            bool operator()(const Glyph& g0, const Glyph& g1) const { return g0.index == g1.index && g0.height == g1.height; }
        };
        std::unordered_set<Glyph, Glyph, Glyph> glyphs;
        bool initInProgress;

        std::unordered_set<Glyph, Glyph, Glyph>::iterator getGlyph(int index, int height, Atlas& atlas);
    };

}
