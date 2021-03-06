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
        WidgetTemplate(Widget* parent);

        static TypeWidget& getType();

        MLParser& getParser() { return parser; }

        // polymorphic interface
        virtual Identifier baseType() const final override { return Identifier::Template; }
        virtual bool setData(int iTpl, int fTpl) final override;

    public:
        MLParser parser;
    };

}
