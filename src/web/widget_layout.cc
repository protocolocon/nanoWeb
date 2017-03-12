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
        Identifier::LayoutVer, sizeof(WidgetLayout), {
            { Identifier::margin,         PROP(WidgetLayout, margin,     Float,        4, 0, 0) },
        }
    };

}

namespace webui {

    WidgetLayout::WidgetLayout(Widget* parent, int coord):
        Widget(parent), margin(0), scrolling(false), coord(coord), position(1e10f), dragDrop(nullptr) {
        typeWidget = coord ? &widgetLayoutVerType : &widgetLayoutHorType;
    }

    TypeWidget& WidgetLayout::getTypeHor() {
        return widgetLayoutHorType;
    }

    TypeWidget& WidgetLayout::getTypeVer() {
        return widgetLayoutVerType;
    }

    void WidgetLayout::render(int alphaMult) {
        alphaMult = renderBase(alphaMult);
        if (alphaMult) {
            // render only visible widgets taking into account they are sorted (use renderVisibilityBox intersection)
            Box4f boxVisibilitySave(Context::renderVisibilityBox);
            Context::renderVisibilityBox.intersect(box);

            float maxCoord(Context::renderVisibilityBox.pos[coord] + Context::renderVisibilityBox.size[coord]);
            for (auto* child: children) {
                if (child->box.pos[coord] >= maxCoord) break;
                child->render(alphaMult);
            }
            if (dragDrop) {
                alphaMult = Context::render.multAlpha(alphaMult, alpha);
                dragDrop->translate(Input::cursor - Input::cursorLeftPress);
                dragDrop->render(alphaMult >> 1);
                dragDrop->translate(Input::cursorLeftPress - Input::cursor);
            }

            Context::renderVisibilityBox = boxVisibilitySave;
        }
    }

    bool WidgetLayout::input() {
        // scroll content
        if (scrollable && Context::cursor == Cursor::Hand && Input::mouseButtonAction &&
            Input::mouseButton == GLFW_MOUSE_BUTTON_LEFT && Input::mouseAction == GLFW_PRESS) {
            // this widge takes care of the event and does not propagate upwards
            Input::mouseButtonAction = false;
            Input::mouseButtonWidget = this;
            Input::cursorLeftPress[coord] -= position;
            scrolling = true;
        } else if (scrolling) {
            if (Input::mouseButton == GLFW_MOUSE_BUTTON_LEFT && Input::mouseAction == GLFW_RELEASE) {
                scrolling = false;
            } else if (Input::mouseAction) {
                position = Input::cursor[coord] - Input::cursorLeftPress[coord];
                return true;
            }
        }

        // drag & drop
        if (Input::mouseButtonWidget && Input::cursorLeftPress.manhatan(Input::cursor) > 16) {
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
        box = boxAvail;
        if (visible && box.size[coord] >= 0) {
            // margin (only once)
            if (position == 1e10f) position = margin;
            int coord2(coord ^ 1);
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
            LinearArrangement<float>::Elem elems[children.size() + 1];
            LinearArrangement<float> la(elems, boxAvail.pos[coord] + position);
            for (auto child: children)
                if (child->isVisible())
                    la.add(child->box.size[coord], child->size[coord].get(boxAvail.size[coord]), child->size[coord].relative);
                else
                    la.add(child->box.size[coord], 0, true);
            stable = la.calculate(boxAvail.size[coord]);

            float lastCoord(la.get(0));
            for (size_t idx = 0; idx < children.size(); idx++) {
                auto* child(children[idx]);
                Box4f b;
                float newCoord = la.get(idx + 1);
                // calculate children box
                b.pos[coord]   = lastCoord;
                b.pos[coord2]  = box.pos[coord2];
                b.size[coord]  = newCoord - lastCoord;
                b.size[coord2] = child->size[coord2].get(box.size[coord2]);
                lastCoord      = newCoord;

                // recurse layout
                stable &= child->layout(b);
            }
            // adaptative size
            float requiredSize(elems[children.size()].posTarget - elems[0].posTarget);
            if (size[coord].adapt) {
                size[coord].size = requiredSize; // total space required
                scrollable = 0;
            } else {
                scrollable = box.size[coord] + 1.1f < requiredSize;
                // converge position
                if (!scrolling) {
                    float targetPosition(position);
                    if (targetPosition > margin) targetPosition = margin;
                    else if (targetPosition < margin && targetPosition < box.size[coord] - requiredSize - margin)
                        targetPosition = box.size[coord] - requiredSize - margin;
                    stable &= ctx.getCloser(position, targetPosition);
                }
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
