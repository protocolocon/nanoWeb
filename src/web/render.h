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
        inline void roundedRect(float x, float y, float w, float h, float r) const { nvgRoundedRect(vg, x, y, w, h, r); }
        inline void fillColor(RGBA color) const { nvgFillColor(vg, color.toVGColor(alpha)); nvgFill(vg); }
        inline void fillVertGrad(float y, float h, RGBA top, RGBA bottom) const {
            NVGpaint bg(nvgLinearGradient(vg, 0, y, 0, y + h, top.toVGColor(alpha), bottom.toVGColor(alpha)));
            nvgFillPaint(vg, bg);
            nvgFill(vg);
        }
        inline void strokeWidth(float width) const { nvgStrokeWidth(vg, width); }
        inline void strokeColor(RGBA color) const { nvgStrokeColor(vg, color.toVGColor(alpha)); }
        inline void stroke() const { nvgStroke(vg); }

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
