/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include "stb_truetype.h"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace render {

    class Atlas;
    class VertexBuffer;

    class Font {
    public:
        Font();
        ~Font();

        bool initialized() const { return info.data; }

        // font memory has to be kept and will be released in the destructor
        bool init(uint8_t* font);

        void text(const char* text, const char* textEnd, int height, Atlas& atlas, VertexBuffer& vertex);

    private:
        stbtt_fontinfo info;
        std::unordered_map<uint32_t, int> codepointIndex; // caches codepoint to glyph index map
        struct Glyph {
            Glyph() { }
            Glyph(int index, int height, int atlas = -1): index(index), height(height), atlas(atlas) { }

            int index;   // index in the font
            int height;  // height of the font
            int atlas;   // index in atlas
            size_t operator()(const Glyph& g) const { return g.index + g.height * 61; }
            bool operator()(const Glyph& g0, const Glyph& g1) const { return g0.index == g1.index && g0.height == g1.height; }
        };
        std::unordered_set<Glyph, Glyph, Glyph> glyphs;
    };

}
