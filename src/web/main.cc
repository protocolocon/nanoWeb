/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "compatibility.h"
#include "main.h"
#include "input.h"
#include <cmath>
#include <cassert>
#include <sys/time.h>

using namespace std;

namespace webui {

    Context::Context(): app(*this), renderForced(true) {
        if (!render.init()) {
            LOG("cannot initialize render");
            assert(false && "cannot initialize render");
        }
        Input::init(render.getWin(), &app);
        updateTime();
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
        LOG("canvas resize %d x %d", width, height);
        forceRender();
        render.setWindowSize(width, height);
        app.resize(width, height);
    }

    void Context::updateTime() {
        struct timeval now;
        gettimeofday(&now, nullptr);
        timeDiffUs = -timeUs;
        timeUs = now.tv_sec * 1000000 + now.tv_usec;
        timeDiffUs += timeUs;
        timeRatio = int(65536.f * powf(0.96f, float(timeDiffUs >> 9)));
        time1MRatio = 65536 - timeRatio;
    }

    Context ctx;

    inline void mainIteration() {
        ctx.mainIteration();
    }

    extern "C" {
        void javascriptCanvasResize(int width, int height) {
            ctx.resize(width, height);
        }
    }

}

int main(int argc, char* argv[]) {
    // main loop
    webui::setMainLoop(argc, argv, webui::mainIteration);
    return 0;
}
