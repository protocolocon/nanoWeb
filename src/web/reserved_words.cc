/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "reserved_words.h"
#include "context.h"
#include "compatibility.h"
#include "string_manager.h"
#include <cassert>

#define _ "\0"

namespace {

    const char* reservedWords =
        "Application" _
        "Widget" _
        "LayoutHor" _
        "LayoutVer" _
        "Template" _
        "Timer" _
        "WLast" _

        "id" _
        "x" _
        "y" _
        "w" _
        "h" _
        "width" _
        "height" _
        "background" _
        "all" _
        "visible" _
        "inside" _
        "canFocus" _
        "active" _
        "draggable" _
        "color" _
        "onEnter" _
        "onLeave" _
        "onClick" _
        "onRender" _
        "onRenderActive" _
        "onTimeout" _
        "define" _
        "self" _
        "repeat" _
        "delay" _
        "parent" _
        "ALast" _

        "propInt16" _
        "propFloat" _
        "propText" _
        "propColor" _
        "propId" _
        "PLast" _

        "log" _
        "beginPath" _
        "moveto" _
        "lineto" _
        "bezierto" _
        "closePath" _
        "roundedRect" _
        "fillColor" _
        "fillVertGrad" _
        "fill" _
        "strokeWidth" _
        "strokeColor" _
        "stroke" _
        "font" _
        "text" _
        "textLeft" _
        "translate" _
        "scale" _
        "resetTransform" _
        "set" _
        "scissor" _
        "query" _
        "triggerTimers" _
        "CLast" _

        "+" _
        "-" _
        "*" _
        "/" _
        "%" _
        "=" _
        "FLast" _
        _;
}

namespace webui {

    void addReservedWords() {
        const char* rw(reservedWords);
        while (*rw) {
            int n(strlen(rw));
            Context::strMng.add(rw);
            rw += n + 1;
        }
        assert(Context::strMng.search("query").getId() == Identifier::query);
        assert(Context::strMng.search("+").getId() == Identifier::add);
        assert(Context::strMng.search("-").getId() == Identifier::sub);
        assert(Context::strMng.search("=").getId() == Identifier::assign);
    }

}
