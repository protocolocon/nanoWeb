/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "font.h"
#include "atlas.h"
#include "vertex.h"
#include <cmath>

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

    Font::Font(): initInProgress(false) {
    }

    bool Font::initializationInProgress() {
        return initInProgress;
    }

    void Font::markInitInProgress() {
        initInProgress = true;
    }

    void Font::text(const char* text, const char* textEnd, int height, Atlas& atlas, VertexBuffer& vertex) {
	uint32_t utf8state = 0;
	uint32_t codepoint;
        float x(40.f), y(40.f);
        float scale(scaleHeight(height));
        for (; text < textEnd; ++text) {
            if (fons__decutf8(&utf8state, &codepoint, *(const uint8_t*)text)) continue;

            // get index
            auto codepointIt(codepointIndex.find(codepoint));
            if (codepointIt == codepointIndex.end()) {
                int index(glyphIndex(codepoint));
                codepointIt = codepointIndex.insert(make_pair(codepoint, index)).first;
            }

            // get glyph
            auto glyphIt(getGlyph(codepointIt->second, height, atlas));

            // add vertices
            const auto& sprite(atlas.get(glyphIt->atlas));
            vertex.addQuad({
                    x + glyphIt->x0,                             y + glyphIt->y0,
                    x + glyphIt->x0 + float(sprite.box.width()), y + glyphIt->y0 + float(sprite.box.height()) }, sprite.tex(), RGBA(0x80000000));

            // advance to next position
            x += floorf(float(glyphIt->advance) * scale + 0.5f);
        }
    }

    unordered_set<Font::Glyph, Font::Glyph, Font::Glyph>::iterator Font::getGlyph(int index, int height, Atlas& atlas) {
        auto glyphIt(glyphs.find(Glyph(index, height)));
        if (glyphIt == glyphs.end()) {
            // get font metrics
            int advance, lsb, ix0, iy0, ix1, iy1;
            glyphParams(index, height, &advance, &lsb, &ix0, &iy0, &ix1, &iy1);

            // need to create
            LOG("Index=%d H=%d Glyph=(%d %d) %d %d %d %d", index, height, ix1 - ix0, iy1 - iy0, advance, ix0, iy0, lsb);
            glyphIt = glyphs.insert(Glyph(index, height, atlas.add(ix1 - ix0, iy1 - iy0), advance, ix0, iy0)).first;
            atlas.addBitmap(glyphIt->atlas, glyphBitmap());
        }
        return glyphIt;
    }

    void Font::populate(int height, Atlas& atlas) {
        for (int index = 0; index < 2000; index++) getGlyph(index, height, atlas);
    }

}
