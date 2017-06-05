/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "font_ll.h"
#include <cassert>
#include <cstdlib>
#ifdef USE_STB_TT
#  define STB_TRUETYPE_IMPLEMENTATION
#  include "stb_truetype.h"
#endif

namespace {

    UFT(
        struct FontLLInit {
            FontLLInit() {
                FT_Error ftError;
                ftError = FT_Init_FreeType(&ftLibrary);
                if (ftError) LOG("error: freetype init");
            }
            FT_Library ftLibrary;
        };
        FontLLInit fontLLInit;
    );

}

namespace render {

    FontLL::FontLL(): fontData(nullptr), bitmap(0), freeBitmap(false) {
    }

    FontLL::~FontLL() {
        free(fontData);
        if (freeBitmap) free(bitmap);
    }

    bool FontLL::init(uint8_t* data, int size) {
        assert(!fontData);
        fontData = data;

        DIAG(
            UST(
                for (int index = 0; ; index++) {
                    auto off(stbtt_GetFontOffsetForIndex(data, index));
                    if (off < 0) break;
                    LOG("UST: Subfont %d at %d", index, off);
                });
            );

        UFT(
            FT_Error ftError(FT_New_Memory_Face(fontLLInit.ftLibrary, data, size, 0/*index*/, &face));
            if (ftError) {
                LOG("UFT: cannot initialize font");
                return false;
            });
        UST(
            if (!stbtt_InitFont(&info, data, 0/*offset*/)) {
                LOG("UST: cannot initialize font");
                free(data);
                return false;
            });
        return true;
    }

    int FontLL::glyphIndex(uint32_t codepoint) {
        UFT(return FT_Get_Char_Index(face, codepoint));
        UST(return stbtt_FindGlyphIndex(&info, codepoint));
        return 0;
    }

    float FontLL::scaleHeight(int heightPixels) {
        UFT(return float(heightPixels) / (face->ascender - face->descender));
        UST(return stbtt_ScaleForPixelHeight(&info, heightPixels));
        return 0;
    }

    int FontLL::kerning(int glyphA, int glyphB) {
        UFT(FT_Vector ftKerning;
            FT_Get_Kerning(face, glyphA, glyphB, FT_KERNING_DEFAULT, &ftKerning);
            return (int)((ftKerning.x + 32) >> 6));
        UST(return stbtt_GetGlyphKernAdvance(&info, glyphA, glyphB));
        return 0;
    }

    bool FontLL::glyphParams(int glyph, int heightPixels, int* advance, int* lsb, int* x0, int* y0, int* x1, int* y1) {
        if (freeBitmap) free(bitmap);
        UFT(
            FT_Fixed advFixed;
            if (FT_Set_Pixel_Sizes(face, 0, FT_UInt(float(heightPixels) * float(face->units_per_EM) /
                                                    float(face->ascender - face->descender))) ||
                FT_Load_Glyph(face, glyph, FT_LOAD_RENDER) ||
                FT_Get_Advance(face, glyph, FT_LOAD_NO_SCALE, &advFixed)) return false;

            auto ftGlyph = face->glyph;
            *advance = int(advFixed);
            *lsb = int(ftGlyph->metrics.horiBearingX);
            *x0 = ftGlyph->bitmap_left;
            *x1 = *x0 + ftGlyph->bitmap.width;
            *y0 = -ftGlyph->bitmap_top;
            *y1 = *y0 + ftGlyph->bitmap.rows;
            bitmap = face->glyph->bitmap.buffer;
            freeBitmap = false;
            return true;
        );
        UST(
            float scale(scaleHeight(heightPixels));
            stbtt_GetGlyphHMetrics(&info, glyph, advance, lsb);
            stbtt_GetGlyphBitmapBox(&info, glyph, scale, scale, x0, y0, x1, y1);
            bitmap = stbtt_GetGlyphBitmap(&info, scale, scale, glyph, 0, 0, 0, 0);
            freeBitmap = true;
            return true;
        );
        return false;
    }

}
