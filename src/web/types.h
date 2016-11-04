/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "nanovg.h"
#include "compatibility.h"
#include <string>
#include <cstdlib>

namespace webui {

    class RGBA {
    public:
        RGBA(): c(0) { }
        RGBA(uint32_t c): c(c) { }
        RGBA(const char* str) {
            char* end;
            c = strtol(str + 2/*skip '"#' */, &end, 16);
            if (end - str - 2 == 6) c = c << 8 | 0xff; // alpha
        }

        inline uint8_t r() const { return  c >> 24; }
        inline uint8_t g() const { return (c >> 16) & 0xff; }
        inline uint8_t b() const { return (c >>  8) & 0xff; }
        inline uint8_t a() const { return (c >>  0) & 0xff; }

        inline float rf() const { return u2f(r()); }
        inline float gf() const { return u2f(g()); }
        inline float bf() const { return u2f(b()); }
        inline float af() const { return u2f(a()); }

        inline uint32_t rgba() const { return c; }

        inline NVGcolor toVGColor() const { return NVGcolor {{{ rf(), gf(), bf(), af() }}}; }
        inline NVGcolor toVGColor(int alpha) const { return NVGcolor {{{ rf(), gf(), bf(), float(a() * alpha) * (1.0f / 65535.0f) }}}; }

    private:
        uint32_t c;

        inline static float u2f(uint8_t v) { return float(v) * (1.0f / 255.0f); }
    };


    class SizeRelative {
    public:
        inline SizeRelative(float x, bool rel) { assign(x, rel); }
        inline void assign(float x, bool rel) { size = rel ? x * 2.56f + 0.5f : x; relative = rel; }
        inline void operator=(const std::pair<const char*, int>& ss) {
            if ((relative = ss.first[ss.second - 1] == '%')) size = int(atof(ss.first) * 2.56f + 0.5f); else size = atoi(ss.first);
        }
        inline int get(int absolute) const { return relative ? (absolute * int(size)) >> 8 : size; }

    public:
        int16_t size:15;
        uint16_t relative:1;
    };

}
