/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "render.h"
#include "application.h"
#include "compatibility.h"
#include <cassert>

using namespace std;

namespace webui {

    class Context {
    public:
        Context(): renderForced(true) {
            if (!render.init()) {
                cout << "cannot initialize render" << endl;
                assert(false && "cannot initialize render");
            }
        }

        inline void forceRender() {
            renderForced = true;
        }

        void mainIteration() {
            app.refresh();
            if (renderForced) {
                renderForced = false;
                LOG("render");
                glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                render.swapBuffers();
            }
        }

        inline Render& getRender() { return render; }

    private:
        Render render;
        Application app;
        bool renderForced;
    };

    Context ctx;

    inline void mainIteration() {
        ctx.mainIteration();
    }

    extern "C" {
        void javascriptCanvasResize(int width, int height) {
            ctx.forceRender();
            cout << "canvas resize: " << width << ' ' << height << endl;
            ctx.getRender().setWindowSize(width, height);
        }
    }

}

int main(int argc, char* argv[]) {
    // main loop
    webui::setMainLoop(argc, argv, webui::mainIteration);
    return 0;
}
