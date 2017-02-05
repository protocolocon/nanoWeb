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

    class Widget;

    class RGBA {
    public:
        RGBA(): c(0) { }
        RGBA(uint32_t c): c(c) { }
        RGBA(const char* str) {
            char* end;
            c = strtoul(str + 2/*skip '"#' */, &end, 16);
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

        void multRGB(int x) { rr = mult(rr, x); gg = mult(gg, x); bb = mult(bb, x); }
        static inline uint8_t mult(uint8_t v, int mult) { return clamp((uint32_t(v) * mult) >> 8); }
        static inline uint8_t clamp(int v) { return v > 255 ? 255 : v; }

    protected:
        union {
            uint32_t c;
            struct {
                uint8_t aa, bb, gg, rr;
            };
        };

        inline static float u2f(uint8_t v) { return float(v) * (1.0f / 255.0f); }
    };


    class RGBAref: public RGBA {
    public:
        RGBAref(): RGBA() { }
        RGBAref(uint32_t c): RGBA(c) { }
        RGBAref(RGBA rgba): RGBA(rgba.rgba() & -2) { }
        RGBAref(const char* str): RGBA(str) { c &= -2; }
        RGBAref(int ref, float value) { assign(ref, value); }
        void assign(int ref, float value);
        RGBA get(const Widget* widget) const;
    };


    class SizeRelative {
    public:
        inline SizeRelative(float x, bool rel) { assign(x, rel); }
        void assign(float x, bool rel);
        inline void operator=(const std::pair<const char*, int>& ss) { assign(atof(ss.first), ss.first[ss.second - 1] == '%'); }
        inline int get(int absolute) const { return relative ? (absolute * int(size)) >> 8 : size; }
        float dumpValue() const;
        char dumpFlags() const;

    public:
        uint16_t size:14;
        uint16_t relative:1;
        uint16_t adapt:1;
    };


    class TextPropOrStrId {
    public:
        TextPropOrStrId(StringId id): off(int(id.getId())), prop(false) { }
        TextPropOrStrId(int propPos): off(propPos), prop(true) { }
        const char* get(Widget* w, StringManager& strMng) {
            if (prop) {
                const char* str(reinterpret_cast<const char**>(w)[off]);
                return str ? str : "";
            } else
                return strMng.get(StringId(off));
        }

    private:
        uint32_t off:31;
        uint32_t prop:1;  // 0: strId   1: Text property
    };

}
