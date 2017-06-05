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
#include "client_app.h"
#include "communication.h"
#include <cassert>

using namespace std;

namespace render {

    Context::Context(): renderForced(true), timeMs(getTimeNowMs()) {
    }

    bool Context::initialize(DIAG(bool initRender, bool requestAppDescription)) {
        DIAG(if (initRender)) {
            if (!render.init()) {
                LOG("cannot initialize render");
                assert(false && "cannot initialize render");
                return false;
            }
        }

        if (!vertexBuffer.init()) {
            LOG("cannot initialize vertex buffer");
            return false;
        }

        if (!atlas.init()) {
            LOG("cannot initialize atlas");
            return false;
        }

        // test render code (atlas)
        vertexBuffer.clear();
        vertexBuffer.addQuad({ 0.f, 0.f, 1024.f, 1024.f }, { 0, 0, 0xffff, 0xffff }, RGBA(0x80000000));

        updateTime();
        return true;
    }

    void Context::mainIteration() {
        // calculate time / and frame offset
        updateTime();

        comm.refresh();

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
    vector<Font> fonts;
    VertexBuffer vertexBuffer;
    ChunkedCommunication comm;
    ClientApp* app = nullptr;
    function<void ()> appOnInit = []() { return true; };
    function<void ()> appOnConnected = []() { };
    function<void (char* message, int size)> appOnReceiveMessage = [](char* message, int size) { };
    function<void ()> appOnReceiveResource = []() { };

}
