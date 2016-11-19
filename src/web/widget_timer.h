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
        inline WidgetTimer(Widget* parent = nullptr): Widget(parent), curDelay(0x7fffffff), delay(1000), repeat(1) {
            size[0].assign(0, false);
            size[1].assign(0, false);
        }

        // polymorphic interface
        virtual Identifier type() const final override { return Identifier::Timer; }
        virtual const Properties& getProps() const final override;

        bool refreshTimer(Context& ctx); // returns true on command execution

    public:
        int curDelay;

        int delay;
        uint8_t repeat;
    };

}
