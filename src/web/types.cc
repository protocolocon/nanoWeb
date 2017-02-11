/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "types.h"

using namespace std;

namespace webui {

    // class SizeRelative
    void SizeRelative::assign(float x, bool rel) {
        size = rel ? x * 2.56f + 0.5f : x;
        if (size) {
            relative = rel;
            adapt = false;
        } else {
            relative = false;
            adapt = true;
        }
    }

    float SizeRelative::dumpValue() const {
        if (relative)
            return float(size) * (100.0f / 256.0f);
        else
            return float(size);
    }

    char SizeRelative::dumpFlags() const {
        if (relative) return '%';
        if (adapt) return '+';
        return ' ';
    }

}
