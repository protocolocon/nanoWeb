/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_application.h"
#include "nanovg.h"
#include "context.h"
#include "application.h"
#include <cassert>

using namespace std;

namespace webui {

    void WidgetApplication::render(Context& ctx, int alphaMult) {
        assert(visible);
        glClearColor(background.rf(), background.gf(), background.bf(), background.af());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        auto& app(ctx.getApplication());
        const auto& actionTable(app.getActionTable(actions));
        if (actionTable.onRender)
            app.executeNoCheck(actionTable.onRender, this);

        for (auto* child: children) child->render(ctx, alphaMult);
    }

}
