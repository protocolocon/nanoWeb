/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "context.h"
#include "render.h"
#include <cassert>

namespace render {

    Context::Context(): renderForced(true), timeMs(getTimeNowMs()) {
    }

    void Context::initialize(DIAG(bool initRender, bool requestAppDescription)) {
        DIAG(if (initRender)) {
            if (!render.init()) {
                LOG("cannot initialize render");
                assert(false && "cannot initialize render");
            }
        }
        updateTime();
    }

    void Context::mainIteration() {
        // calculate time / and frame offset
        updateTime();

        if (!serverRefresh()) {
            // server context is lost
            LOG("server communication restarted");
        }
        if (serverWrite((uint8_t*)"test", 4) != 4)
            LOG("cannot send");

        int nBuffer;
        uint8_t buffer[16];
        if ((nBuffer = serverRead(buffer, sizeof(buffer))))
            LOG("%.*s", nBuffer, buffer);

        // render if required
        if (renderForced) {
            renderForced = false;
            render.swapBuffers();
        }
    }

    void Context::resize(int width, int height) {
        DIAG(LOG("canvas resize %d x %d", width, height));
        forceRender();
        render.setWindowSize(width, height);
    }

    void Context::updateTime() {
        timeDiffMs = -timeMs;
        timeMs = getTimeNowMs();
        timeDiffMs += timeMs;
    }

    Context ctx;
    Render render;

}
