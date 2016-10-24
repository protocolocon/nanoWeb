/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <string>

namespace webui {

    class RGBA {
    public:
        RGBA(): c(0) { }
        RGBA(const std::string& str): c(strtol(str.c_str() + 2/*skip '"#' */, nullptr, 16)) {
            if (str.size() - 3 == 6) c = c << 8 | 0xff; // alpha
        }

        inline uint8_t r() const { return  c >> 24; }
        inline uint8_t g() const { return (c >> 16) & 0xff; }
        inline uint8_t b() const { return (c >>  8) & 0xff; }
        inline uint8_t a() const { return (c >>  0) & 0xff; }

    private:
        uint32_t c;
    };

}
