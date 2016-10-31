/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "types.h"
#include "vector.h"
#include "nanovg.h"

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
        inline int multAlpha(int m, int a) { return alpha = (m * a) >> 8; }
        inline void beginPath() const { nvgBeginPath(vg); }
        inline void roundedRect(int x, int y, int w, int h, int r) const { nvgRoundedRect(vg, x, y, w, h, r); }
        inline void fillColor(RGBA color) const { nvgFillColor(vg, color.toVGColor(alpha)); nvgFill(vg); }
        inline void strokeColor(RGBA color) const { nvgStrokeColor(vg, color.toVGColor(alpha)); nvgStroke(vg); }

        // getters
        inline int getWidth() const { return windowSize[0]; }
        inline int getHeight() const { return windowSize[1]; }
        inline GLFWwindow* getWin() { return win; }
        inline NVGcontext* getVg() { return vg; }

    private:
        GLFWwindow* win;
        NVGcontext* vg;
        V2s windowSize;
        int alpha;
    };

}
