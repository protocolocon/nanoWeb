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

    bool WidgetLayoutHor::layout(V2s posAvail, V2s sizeAvail, float time) {
        curPos = posAvail;
        curSize = sizeAvail;

        // calculate sizes
        bool resizable[children.size()];
        int sizes[children.size()];
        int resizables(0), total(0);
        for (size_t idx = 0; idx < children.size(); idx++) {
            resizable[idx] = children[idx]->isWidthRelative();
            sizes[idx] = children[idx]->getWidthTarget(sizeAvail.x);
            total += sizes[idx];
            if (resizable[idx]) resizables += sizes[idx];
        }
        // reduce resizable widgets
        if (total > sizeAvail.x) {
            float ratio(float(sizeAvail.x - total + resizables) / float(resizables));
            for (size_t idx = 0; idx < children.size(); idx++) {
                if (resizable[idx])
                    sizes[idx] = int(float(sizes[idx]) * ratio + 0.5f);
            }
        }
        // position each widget
        short pos(posAvail.x);
        bool stable(true);
        for (size_t idx = 0; idx < children.size(); idx++) {
            stable &= children[idx]->layout(V2s(pos, posAvail.y), V2s(sizes[idx], children[idx]->getHeightTarget(curSize.y)), time);
            pos += sizes[idx];
        }
        return stable;
    }

}
