/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_layout_hor.h"
#include "main.h"
#include "nanovg.h"

using namespace std;

namespace webui {

    bool WidgetLayoutHor::layout(Context& ctx, V2s posAvail, V2s sizeAvail) {
        bool stable(true);
        if (visible) {
            curPos = posAvail;
            curSize = sizeAvail;

            // use linear arrangement util
            auto& la(ctx.getLinearArrangement());
            la.init(posAvail.x, children.size());
            for (auto child: children)
                if (child->isVisible())
                    la.add(child->getWidth(), child->getWidthTarget(sizeAvail.x), child->isWidthRelative());
                else
                    la.add(child->getWidth(), 0, true);
            stable = la.calculate(ctx, sizeAvail.x);
            for (size_t idx = 0; idx < children.size(); idx++) {
                auto* child(children[idx]);
                stable &= child->layout(ctx, V2s(la.get(idx), posAvail.y), V2s(la.get(idx + 1) - la.get(idx), child->getHeightTarget(curSize.y)));
            }
        }
        return stable;
    }

}
