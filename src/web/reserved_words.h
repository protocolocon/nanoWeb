/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <cstring>

#define OffsetEnum(x) x + constlen(#x)

namespace webui {

    void addReservedWords();

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
        canFocus         = OffsetEnum(inside),
        active           = OffsetEnum(canFocus),
        draggable        = OffsetEnum(active),
        color            = OffsetEnum(draggable),
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
        propInt16        = OffsetEnum(delay),
        propText         = OffsetEnum(propInt16),
        propColor        = OffsetEnum(propText),
        ALast            = OffsetEnum(propColor),

        // commands
        log              = OffsetEnum(ALast),
        toggleVisible    = OffsetEnum(log),
        beginPath        = OffsetEnum(toggleVisible),
        moveto           = OffsetEnum(beginPath),
        lineto           = OffsetEnum(moveto),
        bezierto         = OffsetEnum(lineto),
        closePath        = OffsetEnum(bezierto),
        roundedRect      = OffsetEnum(closePath),
        fillColor        = OffsetEnum(roundedRect),
        fillVertGrad     = OffsetEnum(fillColor),
        fill             = OffsetEnum(fillVertGrad),
        strokeWidth      = OffsetEnum(fill),
        strokeColor      = OffsetEnum(strokeWidth),
        stroke           = OffsetEnum(strokeColor),
        font             = OffsetEnum(stroke),
        text             = OffsetEnum(font),
        textLeft         = OffsetEnum(text),
        translateCenter  = OffsetEnum(textLeft),
        scale100         = OffsetEnum(translateCenter),
        resetTransform   = OffsetEnum(scale100),
        set              = OffsetEnum(resetTransform),
        query            = OffsetEnum(set),
        CLast            = OffsetEnum(query),

        // functions
        add              = OffsetEnum(CLast),      // +
        sub              = OffsetEnum(CLast) + 2,  // -
        mul              = OffsetEnum(CLast) + 4,  // *
        div              = OffsetEnum(CLast) + 6,  // /
        mod              = OffsetEnum(CLast) + 8,  // %
        assign           = OffsetEnum(CLast) + 10, // =
        iXor             = OffsetEnum(CLast) + 12, // ^=
        FLast            = OffsetEnum(CLast) + 15,
    };

}
