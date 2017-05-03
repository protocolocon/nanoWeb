/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "vector.h"
#include "compatibility.h"

struct GLFWwindow;

namespace render {

    class Render {
    public:
        Render(): win(nullptr) { }
        bool init();
        DIAG(void finish());
        void setWindowSize(int width, int height);
        void swapBuffers();
        static bool checkError();

        // getters
        inline int getWidth() const { return windowSize[0]; }
        inline int getHeight() const { return windowSize[1]; }
        inline GLFWwindow* getWin() { return win; }

    private:
        GLFWwindow* win;
        webui::V2s windowSize;
    };

}
