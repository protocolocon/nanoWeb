/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "types.h"
#include "widget.h"

namespace webui {

    class Context;

    class WidgetLayoutHor: public Widget {
    public:
        inline WidgetLayoutHor(Widget* parent = nullptr): Widget(parent) { }

        // polymorphic interface
        virtual bool layout(V2s pos, V2s size, float time); // returns true if stable
    };

}
