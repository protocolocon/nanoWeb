/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "context.h"
#include "font.h"
#include "atlas.h"
#include "render.h"
#include "vertex.h"
#include "application.h"
#include <cassert>

using namespace std;

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

        if (!vertexBuffer.init())
            LOG("cannot initialize vertex buffer");

        if (!atlas.init())
            LOG("cannot initialize atlas");

        // test render code (atlas)
        vertexBuffer.clear();
        vertexBuffer.addQuad({ 0.f, 0.f, 1024.f, 1024.f }, { 0, 0, 0xffff, 0xffff }, RGBA(0x80000000));

        updateTime();
    }

    void Context::mainIteration() {
        // calculate time / and frame offset
        updateTime();

        app.refresh();

        // render if required
        if (renderForced) {
            renderForced = false;

            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            vertexBuffer.render(GL_TRIANGLES);
            render.swapBuffers();
            //LOG("vertices: %d", vertexBuffer.size());
        }
        renderForced = true;///// !!!!!!
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
    Atlas atlas;
    Render render;
    Application app;
    vector<Font> fonts;
    VertexBuffer vertexBuffer;

}
