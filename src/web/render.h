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
        Render(): win(nullptr), vg(nullptr) { }
        bool init();
        DIAG(void finish());
        void setWindowSize(int width, int height);
        void swapBuffers();
        static bool checkError();

        // nanovg
        void beginFrame();
        void endFrame();
        inline int multAlpha(int m, int a) { int alpha((m * a) >> 8); nvgGlobalAlpha(vg, float(alpha) * (1.0f / 256.0f)); return alpha; }
        inline void beginPath() const { nvgBeginPath(vg); }
        inline void moveto(float x, float y) const { nvgMoveTo(vg, x, y); }
        inline void lineto(float x, float y) const { nvgLineTo(vg, x, y); }
        inline void bezierto(float x1, float y1, float x2, float y2, float x, float y) const { nvgBezierTo(vg, x1, y1, x2, y2, x, y); }
        inline void closePath() const { nvgClosePath(vg); }
        inline void roundedRect(float x, float y, float w, float h, float r) const { nvgRoundedRect(vg, x, y, w, h, r); }
        inline void fillColor(RGBA color) const { nvgFillColor(vg, color.toVGColor()); }
        inline void fillVertGrad(float y, float h, RGBA top, RGBA bottom) const {
            nvgFillPaint(vg, nvgLinearGradient(vg, 0, y, 0, y + h, top.toVGColor(), bottom.toVGColor()));
        }
        inline void fill() const { nvgFill(vg); }
        inline void strokeWidth(float width) const { nvgStrokeWidth(vg, width); }
        inline void strokeColor(RGBA color) const { nvgStrokeColor(vg, color.toVGColor()); }
        inline void stroke() const { nvgStroke(vg); }
        inline void translate(float x, float y) const { nvgTranslate(vg, x, y); }
        inline void scale(float x, float y) const { nvgScale(vg, x, y); }
        inline void resetTransform() const { nvgResetTransform(vg); }
        inline void scissor(float x, float y, float w, float h) const { nvgScissor(vg, x, y, w, h); }
        inline void font(int iFont) const { nvgFontFaceId(vg, iFont); }
        inline void fontSize(float size) const { nvgFontSize(vg, size); }
        void textAlign(int align);
        float text(float x, float y, const char* str);

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
