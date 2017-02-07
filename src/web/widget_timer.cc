/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_timer.h"
#include "context.h"
#include "type_widget.h"

using namespace std;
using namespace webui;

namespace {

    TypeWidget widgetTimerType = {
        Identifier::Timer, sizeof(WidgetTimer), {
            { Identifier::repeat,            PROP(WidgetTimer, repeat,  Uint8,        1, 0, 0) },
            { Identifier::delay,             PROP(WidgetTimer, delay,   Int32,        4, 0, 0) },
            { Identifier::onTimeout,         PROP(WidgetTimer, actions, ActionTable,  4, 0, 1) },
        }
    };

}

namespace webui {

    WidgetTimer::WidgetTimer(Widget* parent): Widget(parent), curDelay(0x7fffffff), delay(1000), repeat(1) {
        size[0].assign(0, false);
        size[1].assign(0, false);
        typeWidget = &widgetTimerType;
    }

    TypeWidget& WidgetTimer::getType() {
        return widgetTimerType;
    }

    bool WidgetTimer::refreshTimer() {
        if (repeat < 2) {
            if (curDelay > delay) curDelay = repeat ? 0 : delay; // if repeated, do first trigger immediately
            curDelay -= ctx.getTimeDiffMs();
            if (curDelay <= 0) {
                curDelay += delay;
                // triggered
                const auto& actionTable(Context::app.getActionTable(actions));
                Context::actions.execute(actionTable.onEnter, this);
                if (!repeat) repeat = 2; // disable timer
            }
        }
        return false;
    }

}
