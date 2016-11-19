/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_timer.h"
#include "main.h"
#include "properties.h"

using namespace std;
using namespace webui;

namespace {

    const Properties widgetTimerProperties = {
        { Identifier::repeat,            PROP(WidgetTimer, repeat,  Uint8,        1, 0, 0) },
        { Identifier::delay,             PROP(WidgetTimer, delay,   Int32,        4, 0, 0) },
        { Identifier::onTimeout,         PROP(WidgetTimer, actions, ActionTable,  4, 0, 1) },
    };

}

namespace webui {

    const Properties& WidgetTimer::getProps() const {
        return widgetTimerProperties;
    }

    bool WidgetTimer::refreshTimer(Context& ctx) {
        if (repeat < 2) {
            if (curDelay > delay) curDelay = delay;
            curDelay -= ctx.getTimeDiffMs();
            if (curDelay < 0) {
                curDelay += delay;
                // triggered
                auto& app(ctx.getApplication());
                const auto& actionTable(app.getActionTable(actions));
                if (actionTable.onEnter)
                    app.executeNoCheck(actionTable.onEnter, this);
                if (!repeat) repeat = 2; // disable timer
            }
        }
        return false;
    }

}
