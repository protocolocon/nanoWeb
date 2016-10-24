/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_application.h"
#include "nanovg.h"

namespace webui {

    void WidgetApplication::render(Context& ctx) {
        LOG("app");
        for (auto* child: children) child->render(ctx);
    }

}
