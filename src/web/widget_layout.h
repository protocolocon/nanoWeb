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

    class WidgetLayout: public Widget {
    public:
        inline WidgetLayout(int coord, Widget* parent = nullptr): Widget(parent), coord(coord), dragDrop(nullptr) { }

        // polymorphic interface
        virtual Identifier type() const final override { return Identifier::LayoutHor; }
        virtual void render(Context& ctx, int alphaMult) final override;
        virtual bool input(Application& app) final override; // returns true if actions were executed (affecting application)
        virtual bool layout(Context& ctx, V2s pos, V2s size) final override; // returns true if stable

    private:
        int coord;
        Widget* dragDrop;
    };

}
