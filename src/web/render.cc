/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "render.h"
#include "compatibility.h"

#include "nanovg.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include <cstddef>
#include <cassert>

using namespace std;

namespace webui {

    void errorCallback(int error, const char* description) {
        cout << "error: glfw " << error << ": " << description << endl;
    }

    bool Render::init() {
        // init glfw
        glfwSetErrorCallback(errorCallback);
        if (!glfwInit()) {
            cout << "error: cannot init glfw" << endl;
            return false;
        }

        // create window
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        int width(defaultWidth()); //EMS(EM_ASM_INT_V(return canvas.width)) DSK(1024));
        int height(defaultHeight()); //EMS(EM_ASM_INT_V(return canvas.height)) DSK(800));
        if (!(win = glfwCreateWindow(width, height, "NanoWeb", nullptr, nullptr))) {
            cout << "error: cannot create glfw window" << endl;
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(win);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // render first frame as soon as possible
        glClear(GL_COLOR_BUFFER_BIT);

        // nanovg
        if (!(vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG))) {
            cout << "Could not init nanovg.\n" << endl;
            return false;
	}
        setWindowSize(width, height);
        return true;
    }

    void Render::finish() {
        glfwTerminate();
    }

    void Render::setWindowSize(int width, int height) {
        windowSize[0] = width;
        windowSize[1] = height;
        glViewport(0, 0, width, height);
    }

    void Render::swapBuffers() {
        glfwSwapBuffers(win);
    }

    bool Render::checkError() {
        GLuint error(glGetError());
        if (error != GL_NO_ERROR) {
            cout << "GL error: " << error << endl;
            assert(false && "checkError sync on GL errors");
            return false;
        }
        return true;
    }

}
