/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "vector.h"

struct NVGcontext;
struct GLFWwindow;

namespace webui {

    class Render {
    public:
        bool init();
        void finish();
        void setWindowSize(int width, int height);
        void swapBuffers();
        static bool checkError();

        // nanovg
        void beginFrame();
        void endFrame();

        // getters
        inline int getWidth() const { return windowSize[0]; }
        inline int getHeight() const { return windowSize[1]; }
        inline NVGcontext* getVg() { return vg; }

    private:
        GLFWwindow* win;
        NVGcontext* vg;
        V2i windowSize;
    };

}
