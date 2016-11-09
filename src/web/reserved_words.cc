/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "reserved_words.h"
#include "compatibility.h"
#include "string_manager.h"
#include <cassert>

#define add(x)                                  \
    strMng.add(#x, constlen(#x) - 1);           \
    assert(strMng.search(#x).getId() ==         \
           int(Identifier::x))

namespace webui {

    void addReservedWords(StringManager& strMng) {
        add(Application);
        add(Widget);
        add(LayoutHor);
        add(WLast);

        add(id);
        add(x);
        add(y);
        add(w);
        add(h);
        add(width);
        add(height);
        add(background);
        add(foreground);
        add(all);
        add(visible);
        add(inside);
        add(color);
        add(onEnter);
        add(onLeave);
        add(onClick);
        add(onRender);
        add(onRenderActive);
        add(define);
        add(ALast);

        add(log);
        add(toggleVisible);
        add(beginPath);
        add(roundedRect);
        add(fillColor);
        add(fillVertGrad);
        add(fill);
        add(strokeWidth);
        add(strokeColor);
        add(stroke);
        add(font);
        add(text);
        add(set);
        add(CLast);
    }

}
