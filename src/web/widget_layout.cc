/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_layout.h"
#include "main.h"
#include "input.h"
#include "nanovg.h"

using namespace std;

namespace webui {

    void WidgetLayout::render(Context& ctx, int alphaMult) {
        Widget::render(ctx, alphaMult);
        if (dragDrop) {
            auto& render(ctx.getRender());
            alphaMult = render.multAlpha(alphaMult, alpha);
            dragDrop->render(ctx, alphaMult);
        }
    }

    bool WidgetLayout::input(Application& app) {
        if (Input::cursorLeftPress.manhatan(Input::cursor) > 16) {
            // drag & drop
            if (!dragDrop) {
                for (size_t idx = 0; idx < children.size(); idx++)
                    if (children[idx] == Input::mouseButtonWidget) {
                        children.erase(children.begin() + idx);
                        dragDrop = Input::mouseButtonWidget;
                        break;
                    }
            }
            if (Input::mouseButton == GLFW_MOUSE_BUTTON_LEFT &&
                Input::mouseAction == GLFW_RELEASE && dragDrop) {
                LOG("push back");
                children.push_back(dragDrop);
                dragDrop = nullptr;
                return true;
            }
            return true;
        }
        return false;
    }

    bool WidgetLayout::layout(Context& ctx, V2s posAvail, V2s sizeAvail) {
        bool stable(true);
        if (visible) {
            curPos = posAvail;
            curSize = sizeAvail;

            // use linear arrangement util
            auto& la(ctx.getLinearArrangement());
            la.init(posAvail[coord], children.size());
            for (auto child: children)
                if (child->isVisible())
                    la.add(child->curSize[coord], child->size[coord].get(sizeAvail[coord]), child->size[coord].relative);
                else
                    la.add(child->curSize[coord], 0, true);
            stable = la.calculate(ctx, sizeAvail[coord]);
            if (!coord)
                for (size_t idx = 0; idx < children.size(); idx++) {
                    auto* child(children[idx]);
                    stable &= child->layout(ctx, V2s(la.get(idx), posAvail.y), V2s(la.get(idx + 1) - la.get(idx), child->size[1].get(curSize.y)));
                }
            else
                for (size_t idx = 0; idx < children.size(); idx++) {
                    auto* child(children[idx]);
                    stable &= child->layout(ctx, V2s(posAvail.x, la.get(idx)), V2s(child->size[0].get(curSize.x), la.get(idx + 1) - la.get(idx)));
                }

            // drag & drop
            if (dragDrop) {
                stable &= dragDrop->layout(ctx, Input::cursor, V2s(dragDrop->size[0].get(curSize.x), dragDrop->size[1].get(curSize.y)));
            }
        }
        return stable;
    }

}
