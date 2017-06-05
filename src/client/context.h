/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include <vector>
#include <functional>

namespace render {

    class Font;
    class Atlas;
    class Render;
    class ClientApp;
    class VertexBuffer;
    class ChunkedCommunication;

    class Context {
    public:
        Context();

        bool initialize(DIAG(bool initRender, bool requestAppDescription));

        void mainIteration();

        void resize(int width, int height);

        inline void forceRender() { renderForced = true; }

    private:
        bool renderForced;
        int timeMs;
        int timeDiffMs;

        void updateTime();
    };

    extern Context ctx;
    extern Atlas atlas;
    extern Render render;
    extern ClientApp* app;
    extern std::vector<Font> fonts;
    extern VertexBuffer vertexBuffer;
    extern ChunkedCommunication comm;
    extern std::function<void ()> appOnInit;
    extern std::function<void ()> appOnConnected;
    extern std::function<void (char* message, int size)> appOnReceiveMessage;
    extern std::function<void ()> appOnReceiveResource;

}
