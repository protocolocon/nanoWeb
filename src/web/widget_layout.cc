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

    void WidgetLayout::render(int alphaMult) {
        Widget::render(alphaMult);
        if (dragDrop) {
            alphaMult = Context::render.multAlpha(alphaMult, alpha);
            dragDrop->translate(Input::cursor - Input::cursorLeftPress);
            dragDrop->render(alphaMult >> 1);
            dragDrop->translate(Input::cursorLeftPress - Input::cursor);
        }
    }

    bool WidgetLayout::input() {
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

    bool WidgetLayout::layout(const Box4f& boxAvail) {
        bool stable(true);
        int coord2(coord ^ 1);
        box = boxAvail;
        if (visible) {
            // drag & drop
            float prevCoord(0);
            if (dragDrop) {
                // remove from list
                for (size_t idx = 0; idx < children.size(); idx++)
                    if (children[idx] == dragDrop)
                        children.erase(children.begin() + idx);
                // find position
                float midCoord(dragDrop->box.pos[coord] + (dragDrop->box.size[coord] * 0.5f) + Input::cursor[coord] - Input::cursorLeftPress[coord]);
                float last(numeric_limits<float>::min());
                size_t pos(children.size());
                for (size_t idx = 0; idx < children.size(); idx++) {
                    auto* child(children[idx]);
                    float mid(child->box.pos[coord] + (child->box.size[coord] * 0.5f));
                    if (midCoord >= last && midCoord < mid) {
                        pos = idx;
                        break;
                    }
                    last = mid;
                }
                // add it in correct position
                children.insert(children.begin() + pos, dragDrop);
                prevCoord = dragDrop->box.pos[coord];
            }

            // use linear arrangement util
            LinearArrangement::Elem elems[children.size() + 1];
            LinearArrangement la(elems, boxAvail.pos[coord]);
            for (auto child: children)
                if (child->isVisible())
                    la.add(child->box.size[coord], child->size[coord].get(boxAvail.size[coord]), child->size[coord].relative);
                else
                    la.add(child->box.size[coord], 0, true);
            stable = la.calculate(boxAvail.size[coord]);
            if (!coord)
                for (size_t idx = 0; idx < children.size(); idx++) {
                    auto* child(children[idx]);
                    stable &= child->layout(Box4f(la.get(idx), boxAvail.pos.y, la.get(idx + 1) - la.get(idx), child->size[1].get(box.size.y)));
                }
            else
                for (size_t idx = 0; idx < children.size(); idx++) {
                    auto* child(children[idx]);
                    stable &= child->layout(Box4f(box.pos.x, la.get(idx), child->size[0].get(box.size.x), la.get(idx + 1) - la.get(idx)));
                }

            // adaptative size
            if (size[coord].adapt) {
                size[coord].size = elems[children.size()].posTarget - elems[0].posTarget;
                if (size[coord].size != box.size[coord]) stable = false;
            }
            if (size[coord2].adapt) {
                if (children.empty())
                    size[coord2].size = 0;
                else
                    size[coord2].size = children[0]->box.size[coord2]; // TODO: take max of all children?
                if (size[coord2].size != box.size[coord2]) stable = false;
            }

            // drag & drop
            if (dragDrop)
                Input::cursorLeftPress[coord] += dragDrop->box.pos[coord] - prevCoord;
        }
        return animeAlpha() && stable;
    }

}
