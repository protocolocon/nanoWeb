/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "compatibility.h"
#include "main.h"
#include "input.h"
#include <cassert>

using namespace std;

namespace webui {

    Context::Context(): app(*this), renderForced(true) {
        if (!render.init()) {
            LOG("cannot initialize render");
            assert(false && "cannot initialize render");
        }
        Input::init(render.getWin(), &app);
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

    void Context::resize(int width, int height) {
        LOG("canvas resize %d x %d", width, height);
        forceRender();
        render.setWindowSize(width, height);
        app.resize(width, height);
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
