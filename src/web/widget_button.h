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

    class WidgetButton: public Widget {
    public:
        inline WidgetButton(Widget* parent = nullptr): Widget(parent) { }

        virtual void render(Context& ctx) override;
        virtual bool set(Identifier id, const std::string& value) override;

    protected:
        RGBA color;
    };

}
