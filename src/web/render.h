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
        inline int multAlpha(int m, int a) { int alpha((m * a) >> 8); nvgGlobalAlpha(vg, float(alpha) * (1.0f / 256.0f)); return alpha; }
        inline void beginPath() const { nvgBeginPath(vg); }
        inline void roundedRect(float x, float y, float w, float h, float r) const { nvgRoundedRect(vg, x, y, w, h, r); }
        inline void fillColor(RGBA color) const { nvgFillColor(vg, color.toVGColor()); }
        inline void fillVertGrad(float y, float h, RGBA top, RGBA bottom) const {
            nvgFillPaint(vg, nvgLinearGradient(vg, 0, y, 0, y + h, top.toVGColor(), bottom.toVGColor()));
        }
        inline void fill() const { nvgFill(vg); }
        inline void strokeWidth(float width) const { nvgStrokeWidth(vg, width); }
        inline void strokeColor(RGBA color) const { nvgStrokeColor(vg, color.toVGColor()); }
        inline void stroke() const { nvgStroke(vg); }
        void font(int iFont, float size);
        void textAlign(int align);
        void text(int x, int y, const char* str);

        int loadFont(const char* name, char* data, int nData) { return nvgCreateFontMem(vg, name, (uint8_t*)data, nData, false); }

        // getters
        inline int getWidth() const { return windowSize[0]; }
        inline int getHeight() const { return windowSize[1]; }
        inline GLFWwindow* getWin() { return win; }
        inline NVGcontext* getVg() { return vg; }

    private:
        GLFWwindow* win;
        NVGcontext* vg;
        V2s windowSize;
    };

}
