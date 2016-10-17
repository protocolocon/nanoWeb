/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

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
        inline int getWidth() const { return windowSize[0]; }
        inline int getHeight() const { return windowSize[1]; }

    private:
        GLFWwindow* win;
        NVGcontext* vg;
        int windowSize[2];
    };

}
