/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "types.h"
#include "widget.h"

namespace webui {

    class WidgetLayout: public Widget {
    public:
        WidgetLayout(Widget* parent, int coord);

        static TypeWidget& getTypeHor();
        static TypeWidget& getTypeVer();

        // polymorphic interface
        virtual Identifier baseType() const final override { return coord ? Identifier::LayoutVer : Identifier::LayoutHor; }
        virtual void render(int alphaMult) final override;
        virtual bool input() final override; // returns true if actions were executed (affecting application)
        virtual bool layout(const Box4f& box) final override; // returns true if stable

    private:
        int coord;
        Widget* dragDrop;
    };

}
