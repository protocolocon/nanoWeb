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
        all              = OffsetEnum(background),
        visible          = OffsetEnum(all),
        inside           = OffsetEnum(visible),
        canFocus         = OffsetEnum(inside),
        active           = OffsetEnum(canFocus),
        draggable        = OffsetEnum(active),
        nodelete         = OffsetEnum(draggable),
        color            = OffsetEnum(nodelete),
        onEnter          = OffsetEnum(color),
        onLeave          = OffsetEnum(onEnter),
        onClick          = OffsetEnum(onLeave),
        onRender         = OffsetEnum(onClick),
        onRenderActive   = OffsetEnum(onRender),
        onTimeout        = OffsetEnum(onRenderActive),
        onHover          = OffsetEnum(onTimeout),
        define           = OffsetEnum(onHover),
        self             = OffsetEnum(define),
        repeat           = OffsetEnum(self),
        delay            = OffsetEnum(repeat),
        parent           = OffsetEnum(delay),
        margin           = OffsetEnum(parent),
        mouseX           = OffsetEnum(margin),
        mouseY           = OffsetEnum(mouseX),
        hoverX           = OffsetEnum(mouseY),
        hoverY           = OffsetEnum(hoverX),
        size             = OffsetEnum(hoverY),
        last             = OffsetEnum(size),
        ALast            = OffsetEnum(last),

        // property attribute definition
        propInt16        = OffsetEnum(ALast),
        propFloat        = OffsetEnum(propInt16),
        propText         = OffsetEnum(propFloat),
        propColor        = OffsetEnum(propText),
        propId           = OffsetEnum(propColor),
        PLast            = OffsetEnum(propId),

        // commands
        log              = OffsetEnum(PLast),
        beginPath        = OffsetEnum(log),
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
        textWidth        = OffsetEnum(textLeft),
        translate        = OffsetEnum(textWidth),
        scale            = OffsetEnum(translate),
        resetTransform   = OffsetEnum(scale),
        set              = OffsetEnum(resetTransform),
        scissor          = OffsetEnum(set),
        resetScissor     = OffsetEnum(scissor),
        query            = OffsetEnum(resetScissor),
        triggerTimers    = OffsetEnum(query),
        CLast            = OffsetEnum(triggerTimers),

        // functions
        add              = OffsetEnum(CLast),      // +
        sub              = OffsetEnum(CLast) + 2,  // -
        mul              = OffsetEnum(CLast) + 4,  // *
        div              = OffsetEnum(CLast) + 6,  // /
        mod              = OffsetEnum(CLast) + 8,  // %
        assign           = OffsetEnum(CLast) + 10, // =
        FLast            = OffsetEnum(CLast) + 15,
    };

}
