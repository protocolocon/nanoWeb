/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "render.h"
#include "compatibility.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include <cstddef>
#include <cassert>

using namespace std;

namespace webui {

    void errorCallback(int error, const char* description) {
        LOG("error: glfw %d: %s", error, description);
    }

    bool Render::init() {
        // init glfw
        glfwSetErrorCallback(errorCallback);
        if (!glfwInit()) {
            LOG("error: cannot init glfw");
            return false;
        }

        // create window
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        int width(defaultWidth());
        int height(defaultHeight());
        if (!(win = glfwCreateWindow(width, height, "NanoWeb", nullptr, nullptr))) {
            LOG("error: cannot create glfw window");
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(win);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // render first frame as soon as possible
        glClear(GL_COLOR_BUFFER_BIT);

        // nanovg
        if (!(vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG))) {
            LOG("Could not init nanovg");
            return false;
	}
        setWindowSize(width, height);
        return true;
    }

    DIAG(void Render::finish() {
            glfwTerminate();
        });

    void Render::setWindowSize(int width, int height) {
        windowSize[0] = width;
        windowSize[1] = height;
        glViewport(0, 0, width, height);
    }

    void Render::beginFrame() {
        nvgBeginFrame(vg, windowSize[0], windowSize[1], 1.0f/*aspect ratio*/);
    }

    void Render::endFrame() {
        nvgEndFrame(vg);
    }

    void Render::swapBuffers() {
        glfwSwapBuffers(win);
    }

    bool Render::checkError() {
        GLuint error(glGetError());
        if (error != GL_NO_ERROR) {
            LOG("GL error: %u", error);
            assert(false && "checkError sync on GL errors");
            return false;
        }
        return true;
    }

    void Render::textAlign(int align) {
        nvgTextAlign(vg, align);
    }

    float Render::text(float x, float y, const char* str) {
        if (!str) return x;
        const char* prev(str);
        while (*str) {
            if (*str == 0x1b) {
                if (prev != str) x = nvgText(vg, x, y, prev, str);
                uint32_t c(uint32_t(str[1]) << 24 | uint32_t(str[2]) << 16 | uint32_t(str[3]) << 8 | uint32_t(str[4])); // for emscripten
                nvgFillColor(vg, RGBA(c << 1).toVGColor());
                str += 5;
                prev = str;
            } else
                ++str;
        }
        return nvgText(vg, x, y, prev, str);
    }

    float Render::textWidth(const char* str) {
        if (!str) return 0.0f;
        const char* prev(str);
        float width(0);
        while (*str) {
            if (*str == 0x1b) {
                if (prev != str) width += nvgTextBounds(vg, 0, 0, prev, str, nullptr);
                str += 5;
                prev = str;
            } else
                ++str;
        }
        return width + nvgTextBounds(vg, 0, 0, prev, str, nullptr);
    }

}
