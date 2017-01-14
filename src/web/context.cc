/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "context.h"
#include "input.h"
#include "compatibility.h"
#include <cmath>
#include <cassert>

using namespace std;

namespace webui {

    Context::Context(): app(*this), renderForced(true), timeMs(getTimeNowMs()) {
        if (!render.init()) {
            LOG("cannot initialize render");
            assert(false && "cannot initialize render");
        }
        Input::init(render.getWin(), &app);
        updateTime();
    }

    Context::~Context() {
        render.finish();
    }

    void Context::initialize() {
        app.initialize();
    }

    void Context::mainIteration() {
        // calculate time / and frame offset
        updateTime();

        // refressh application
        app.refresh();

        // render if required
        if (renderForced) {
            renderForced = false;
            app.render();
            render.swapBuffers();
        }
    }

    void Context::resize(int width, int height) {
        DIAG(LOG("canvas resize %d x %d", width, height));
        forceRender();
        render.setWindowSize(width, height);
        app.resize(width, height);
    }

    void Context::updateTime() {
        timeDiffMs = -timeMs;
        timeMs = getTimeNowMs();
        timeDiffMs += timeMs;
        timeRatio = int(65536.f * powf(0.96f, float(timeDiffMs)));
        time1MRatio = 65536 - timeRatio;
    }

}
