/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_layout.h"
#include "input.h"
#include "nanovg.h"
#include "context.h"
#include "type_widget.h"

using namespace std;
using namespace webui;

namespace {

    TypeWidget widgetLayoutHorType = {
        Identifier::LayoutHor, sizeof(WidgetLayout), { }
    };

    TypeWidget widgetLayoutVerType = {
        Identifier::LayoutVer, sizeof(WidgetLayout), { }
    };

}

namespace webui {

    WidgetLayout::WidgetLayout(Widget* parent, int coord): Widget(parent), coord(coord), dragDrop(nullptr) {
        typeWidget = coord ? &widgetLayoutVerType : &widgetLayoutHorType;
    }

    TypeWidget& WidgetLayout::getTypeHor() {
        return widgetLayoutHorType;
    }

    TypeWidget& WidgetLayout::getTypeVer() {
        return widgetLayoutVerType;
    }

    void WidgetLayout::render(Context& ctx, int alphaMult) {
        Widget::render(ctx, alphaMult);
        if (dragDrop) {
            auto& render(ctx.getRender());
            alphaMult = render.multAlpha(alphaMult, alpha);
            dragDrop->translate(Input::cursor - Input::cursorLeftPress);
            dragDrop->render(ctx, alphaMult >> 1);
            dragDrop->translate(Input::cursorLeftPress - Input::cursor);
        }
    }

    bool WidgetLayout::input(Application& app) {
        if (Input::mouseButtonWidget && Input::cursorLeftPress.manhatan(Input::cursor) > 16) {
            // drag & drop
            if (draggable && !dragDrop) {
                // check if clicked in one of layout widgets
                for (size_t idx = 0; idx < children.size(); idx++)
                    if (children[idx] == Input::mouseButtonWidget) {
                        dragDrop = Input::mouseButtonWidget;
                        break;
                    }
            }
            if (Input::mouseButton == GLFW_MOUSE_BUTTON_LEFT &&
                Input::mouseAction == GLFW_RELEASE && dragDrop) {
                dragDrop = Input::mouseButtonWidget = nullptr;
                return true;
            }
            return true;
        }
        return false;
    }

    bool WidgetLayout::layout(Context& ctx, V2s posAvail, V2s sizeAvail) {
        bool stable(true);
        int coord2(coord ^ 1);
        curPos = posAvail;
        curSize = sizeAvail;
        if (visible) {
            // drag & drop
            short prevCoord(0);
            if (dragDrop) {
                // remove from list
                for (size_t idx = 0; idx < children.size(); idx++)
                    if (children[idx] == dragDrop)
                        children.erase(children.begin() + idx);
                // find position
                short midCoord(dragDrop->curPos[coord] + (dragDrop->curSize[coord] >> 1) + Input::cursor[coord] - Input::cursorLeftPress[coord]);
                short last(numeric_limits<short>::min());
                size_t pos(children.size());
                for (size_t idx = 0; idx < children.size(); idx++) {
                    auto* child(children[idx]);
                    short mid(child->curPos[coord] + (child->curSize[coord] >> 1));
                    if (midCoord >= last && midCoord < mid) {
                        pos = idx;
                        break;
                    }
                    last = mid;
                }
                // add it in correct position
                children.insert(children.begin() + pos, dragDrop);
                prevCoord = dragDrop->curPos[coord];
            }

            // use linear arrangement util
            LinearArrangement::Elem elems[children.size() + 1];
            LinearArrangement la(elems, posAvail[coord]);
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

            // adaptative size
            if (size[coord].adapt) {
                size[coord].size = elems[children.size()].posTarget - elems[0].posTarget;
                if (size[coord].size != curSize[coord]) stable = false;
            }
            if (size[coord2].adapt) {
                if (children.empty())
                    size[coord2].size = 0;
                else
                    size[coord2].size = children[0]->curSize[coord2]; // TODO: take max of all children?
                if (size[coord2].size != curSize[coord2]) stable = false;
            }

            // drag & drop
            if (dragDrop)
                Input::cursorLeftPress[coord] += dragDrop->curPos[coord] - prevCoord;
        }
        return animeAlpha(ctx) && stable;
    }

}
