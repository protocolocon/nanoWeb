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
#include <cmath>

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
        Widget(parent), margin(0), scrolling(false), coord(coord), positionTarget(1e10f), dragDrop(nullptr) {
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
        if (scrollable && Input::keyboardAction && (Input::action == GLFW_PRESS || Input::action == GLFW_REPEAT)) {
            /**/ if (Input::keyButton == GLFW_KEY_HOME)      { positionTarget  = margin; return true; }
            else if (Input::keyButton == GLFW_KEY_END)       { positionTarget  = -1e5f;  return true; }
            else if (Input::keyButton == GLFW_KEY_PAGE_UP)   { positionTarget += 150;    return true; }
            else if (Input::keyButton == GLFW_KEY_PAGE_DOWN) { positionTarget -= 150;    return true; }
            else if (Input::keyButton == GLFW_KEY_UP)        { positionTarget +=  50;    return true; }
            else if (Input::keyButton == GLFW_KEY_DOWN)      { positionTarget -=  50;    return true; }
        }
        if (scrollable && Context::cursor == Cursor::Hand && Input::mouseButtonAction &&
            Input::keyButton == GLFW_MOUSE_BUTTON_LEFT && Input::action == GLFW_PRESS) {
            // this widge takes care of the event and does not propagate upwards
            Input::mouseButtonAction = false;
            Input::mouseButtonWidget = this;
            Input::cursorLeftPress[coord] -= positionTarget;
            scrolling = true;
        } else if (scrolling) {
            if (Input::keyButton == GLFW_MOUSE_BUTTON_LEFT && Input::action == GLFW_RELEASE) {
                scrolling = false;
            } else {
                positionTarget = Input::cursor[coord] - Input::cursorLeftPress[coord];
                return true;
            }
        } else if (scrollable && Input::scroll[coord] != 0.0f) {
            positionTarget -= Input::scroll[coord] * 50;
            return true;
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
            if (Input::keyButton == GLFW_MOUSE_BUTTON_LEFT &&
                Input::action == GLFW_RELEASE && dragDrop) {
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
            if (positionTarget == 1e10f) position = positionTarget = margin;
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
                float diff(0);
                if (positionTarget > margin) diff = margin - positionTarget;
                else if (positionTarget < margin && positionTarget < box.size[coord] - requiredSize - margin)
                    diff = box.size[coord] - requiredSize - margin - positionTarget;
                if (scrolling) {
                    // when scrolling with the mouse, don't separate content too much from limits
                    const float tension(0.7f);
                    float diffTension(diff > 0 ? -powf(diff, tension) : powf(-diff, tension));
                    position = positionTarget + diff + diffTension;
                } else {
                    // scrolling with mouse wheel or converging to position
                    positionTarget += diff;
                    stable &= ctx.getCloser(position, positionTarget);
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
