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

    class WidgetTemplate: public Widget {
    public:
        inline WidgetTemplate(Widget* parent = nullptr): Widget(parent) { }

        // polymorphic interface
        virtual Identifier type() const final override { return Identifier::Template; }

        MLParser& getParser() { return parser; }

    public:
        MLParser parser;
    };

}
