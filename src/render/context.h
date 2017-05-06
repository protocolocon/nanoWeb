/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include <vector>

namespace render {

    class Font;
    class Atlas;
    class Render;
    class Application;
    class VertexBuffer;

    class Context {
    public:
        Context();

        void initialize(DIAG(bool initRender, bool requestAppDescription));

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
    extern Application app;
    extern std::vector<Font> fonts;
    extern VertexBuffer vertexBuffer;

}
