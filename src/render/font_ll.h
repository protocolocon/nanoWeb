/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include <cstdint>

#ifdef USE_FREETYPE
#  include "ft2build.h"
#  include FT_FREETYPE_H
#  include FT_ADVANCES_H
#  define UFT(x, ...)          x, ##__VA_ARGS__
#else
#  define UFT(x, ...)
#endif

#ifdef USE_STB_TT
#  include "stb_truetype.h"
#  define UST(x, ...)          x, ##__VA_ARGS__
#else
#  define UST(x, ...)
#endif

namespace render {

    class FontLL {
    public:
        FontLL();
        ~FontLL();

        bool initialized() const { return fontData; }

        // font memory has to be kept and will be released in the destructor
        bool init(uint8_t* fontData, int fontSize);

        int glyphIndex(uint32_t codepoint);
        float scaleHeight(int heightPixels);
        int kerning(int glyphA, int glyphB);
        bool glyphParams(int glyph, int heightPixels, int* advance, int* lsb, int* x0, int* y0, int* x1, int* y1);
        uint8_t* glyphBitmap() const { return bitmap; }

    private:
        UFT(FT_Face face);
        UST(stbtt_fontinfo info);
        uint8_t* fontData;
        uint8_t* bitmap;
        bool freeBitmap;
    };

}
