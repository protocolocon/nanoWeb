/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "types.h"

using namespace std;

namespace {

    union Modified {
        uint32_t c;
        struct {
            uint32_t modif:1;
            uint32_t op:3;
            uint32_t value:12;
            uint32_t ref:16;
        };
    };


}

namespace webui {

    void RGBAref::assign(int ref, float value) {
        Modified& m(*reinterpret_cast<Modified*>(&c));
        m.c = 0;
        m.modif = 1;
        m.op = 0;
        m.value = int(value * 2.56f);
        m.ref = ref;
    }

    RGBA RGBAref::get(const Widget* widget) const {
        if (c & 1) {
            const Modified& m(*reinterpret_cast<const Modified*>(&c));
            RGBA rgba(reinterpret_cast<const RGBA*>(widget)[m.ref]);
            rgba.multRGB(m.value);
            return rgba;
        } else
            return RGBA(c);
    }

}
