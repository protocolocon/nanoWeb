/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "definition_common.h"
#include "render.h"

IOS(using namespace std);

namespace webui {

    class Context {
    public:
        Context(): renderForced(true) {
            if (!render.init()) {
                IOS(cout << "cannot initialize render" << endl);
                abort();
            }
        }

        inline void forceRender() {
            renderForced = true;
        }

        void mainIteration() {
            if (renderForced) {
                glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            }
        }

        inline Render& getRender() { return render; }

    private:
        Render render;
        bool renderForced;
    };

    Context ctx;

    inline void mainIteration() {
        ctx.mainIteration();
    }

    extern "C" {
        void javascriptCanvasResize(int width, int height) {
            ctx.forceRender();
            IOS(cout << "canvas resize: " << width << ' ' << height << endl);
            ctx.getRender().setWindowSize(width, height);
        }
    }

}

int main() {
    // main loop
    EMS(emscripten_set_main_loop(webui::mainIteration, 0 /* fps */, true /* infinite loop*/));
    DSK(while (true) {
            webui::ctx.mainIteration();
            webui::ctx.getRender().swapBuffers();
            this_thread::sleep_for(chrono::milliseconds(10));
        });

    return 0;
}
