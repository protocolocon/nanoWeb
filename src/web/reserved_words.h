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
        Widget           = OffsetEnum(Application),
        LayoutHor        = OffsetEnum(Widget),
        LayoutVer        = OffsetEnum(LayoutHor),
        Template         = OffsetEnum(LayoutVer),
        Timer            = OffsetEnum(Template),
        WLast            = OffsetEnum(Timer),

        // attributes
        id               = OffsetEnum(WLast),
        x                = OffsetEnum(id),
        y                = OffsetEnum(x),
        w                = OffsetEnum(y),
        h                = OffsetEnum(w),
        width            = OffsetEnum(h),
        height           = OffsetEnum(width),
        background       = OffsetEnum(height),
        foreground       = OffsetEnum(background),
        all              = OffsetEnum(foreground),
        visible          = OffsetEnum(all),
        inside           = OffsetEnum(visible),
        color            = OffsetEnum(inside),
        onEnter          = OffsetEnum(color),
        onLeave          = OffsetEnum(onEnter),
        onClick          = OffsetEnum(onLeave),
        onRender         = OffsetEnum(onClick),
        onRenderActive   = OffsetEnum(onRender),
        onTimeout        = OffsetEnum(onRenderActive),
        define           = OffsetEnum(onTimeout),
        self             = OffsetEnum(define),
        repeat           = OffsetEnum(self),
        delay            = OffsetEnum(repeat),
        ALast            = OffsetEnum(delay),

        // commands
        log              = OffsetEnum(ALast),
        toggleVisible    = OffsetEnum(log),
        beginPath        = OffsetEnum(toggleVisible),
        roundedRect      = OffsetEnum(beginPath),
        fillColor        = OffsetEnum(roundedRect),
        fillVertGrad     = OffsetEnum(fillColor),
        fill             = OffsetEnum(fillVertGrad),
        strokeWidth      = OffsetEnum(fill),
        strokeColor      = OffsetEnum(strokeWidth),
        stroke           = OffsetEnum(strokeColor),
        font             = OffsetEnum(stroke),
        text             = OffsetEnum(font),
        set              = OffsetEnum(text),
        query            = OffsetEnum(set),
        CLast            = OffsetEnum(query),
    };

}
