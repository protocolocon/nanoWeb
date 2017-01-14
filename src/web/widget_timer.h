/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "types.h"
#include "widget.h"
#include "ml_parser.h"

namespace webui {

    class WidgetTimer: public Widget {
    public:
        WidgetTimer(Widget* parent);

        static TypeWidget& getType();

        bool refreshTimer(Context& ctx); // returns true on command execution

        // polymorphic interface
        virtual Identifier baseType() const final override { return Identifier::Timer; }

    public:
        int curDelay;

        int delay;
        uint8_t repeat;
    };

}
