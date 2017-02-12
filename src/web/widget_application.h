/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "types.h"
#include "widget.h"

namespace webui {

    class WidgetApplication: public Widget {
    public:
        WidgetApplication(Widget* parent);

        static TypeWidget& getType();

        // polymorphic interface
        virtual Identifier baseType() const final override { return Identifier::Application; }
        virtual void render(int alphaMult) final override;

    public:
        RGBA background;
    };

}
