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

    class WidgetApplication: public Widget {
    public:
        inline WidgetApplication(Widget* parent = nullptr): Widget(parent), color(-1) { }

        virtual void render(Context& ctx) override;
        virtual bool set(Identifier id, StringManager& strMng, const std::string& value) override;

    protected:
        RGBA color;
    };

}
