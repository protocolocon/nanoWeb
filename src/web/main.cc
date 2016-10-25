/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "main.h"
#include "compatibility.h"
#include <cassert>

using namespace std;

namespace webui {

    Context::Context(): app(*this), renderForced(true) {
        if (!render.init()) {
            LOG("cannot initialize render");
            assert(false && "cannot initialize render");
        }
    }

    void Context::mainIteration() {
        app.refresh();
        if (renderForced) {
            renderForced = false;
            LOG("render");
            app.render();
            render.swapBuffers();
        }
    }

    Context ctx;

    inline void mainIteration() {
        ctx.mainIteration();
    }

    extern "C" {
        void javascriptCanvasResize(int width, int height) {
            ctx.forceRender();
            LOG("canvas resize %d x %d", width, height);
            ctx.getRender().setWindowSize(width, height);
        }
    }

}

int main(int argc, char* argv[]) {
    // main loop
    webui::setMainLoop(argc, argv, webui::mainIteration);
    return 0;
}
