/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <cstring>

#define OffsetEnum(x) x + constlen(#x)

namespace webui {

    class StringManager;

    void addReservedWords(StringManager& strMng);

    template <size_t N>
    constexpr int constlen(const char (& s)[N]) { return N; }

    enum class Identifier: int {
        InvalidId        = -1,

        // widgets
        Application      = 0,
        LayoutHor        = OffsetEnum(Application),
        Button           = OffsetEnum(LayoutHor),
        WLast            = OffsetEnum(Button),

        // attributes
        id               = OffsetEnum(WLast),
        width            = OffsetEnum(id),
        height           = OffsetEnum(width),
        color            = OffsetEnum(height),
        onEnter          = OffsetEnum(color),
        onLeave          = OffsetEnum(onEnter),
        onClick          = OffsetEnum(onLeave),
        ALast            = OffsetEnum(onClick),

        // commands
        log              = OffsetEnum(ALast),
        toggleVisible    = OffsetEnum(log),
        CLast            = OffsetEnum(toggleVisible),
    };

}
