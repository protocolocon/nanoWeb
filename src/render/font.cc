/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "font.h"
#include "atlas.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

using namespace std;

namespace {

#define FONS_UTF8_ACCEPT 0

    // Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
    // See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
    uint32_t fons__decutf8(uint32_t* state, uint32_t* codep, uint32_t byte) {
	static const unsigned char utf8d[] = {
            // The first part of the table maps bytes to character classes that
            // to reduce the size of the transition table and create bitmasks.
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

            // The second part is a transition table that maps a combination
            // of a state of the automaton and a character class to a state.
            0, 12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
            12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
            12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
            12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
            12,36,12,12,12,12,12,12,12,12,12,12,
        };

	unsigned int type = utf8d[byte];

        *codep = (*state != FONS_UTF8_ACCEPT) ?
            (byte & 0x3fu) | (*codep << 6) :
            (0xff >> type) & (byte);

	*state = utf8d[256 + *state + type];
	return *state;
    }

}

namespace render {

    Font::Font() {
        info.data = nullptr;
    }

    Font::~Font() {
        free(info.data);
    }

    bool Font::init(uint8_t* font) {
        DIAG(
            for (int index = 0; ; index++) {
                auto off(stbtt_GetFontOffsetForIndex(font, index));
                if (off < 0) break;
                LOG("Subfont %d at %d", index, off);
            });

        if (!stbtt_InitFont(&info, font, 0/*offset*/)) {
            LOG("cannot initialize font");
            free(font);
            info.data = nullptr;
            return false;
        }
        assert(info.data == font);
        return true;
    }

    void Font::text(const char* text, const char* textEnd, int height, Atlas& atlas, VertexBuffer& vertex) {
	uint32_t utf8state = 0;
	uint32_t codepoint;
        for (; text < textEnd; ++text) {
            if (fons__decutf8(&utf8state, &codepoint, *(const uint8_t*)text)) continue;
            auto codepointIt(codepointIndex.find(codepoint));
            if (codepointIt == codepointIndex.end()) {
                int index(stbtt_FindGlyphIndex(&info, codepoint));
                codepointIt = codepointIndex.insert(make_pair(codepoint, index)).first;
            }

            // get font metrics
            int ix0, iy0, ix1, iy1;
            float scale(STBTT_POINT_SIZE(stbtt_ScaleForPixelHeight(&info, height)));
            stbtt_GetGlyphBitmapBox(&info, codepointIt->second, scale, scale, &ix0, &iy0, &ix1, &iy1);

            LOG("Codepoint=%x Index=%d H=%d -> %.3f", codepoint, codepointIt->second, height, scale);

            // get glyph
            auto glyphIt(glyphs.find(Glyph(codepointIt->second, height)));
            if (glyphIt == glyphs.end()) {
                // need to create
                LOG("Glyph=(%d %d)", ix1 - ix0, iy1 - iy0);
                glyphIt = glyphs.insert(Glyph(codepointIt->second, height, atlas.add(ix1 - ix0, iy1 - iy0))).first;
                auto* bitmap(stbtt_GetGlyphBitmap(&info, scale, scale, codepointIt->second, 0, 0, 0, 0));
                atlas.addBitmap(glyphIt->atlas, bitmap);
                free(bitmap);
            }
        }
    }

}
