/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

// openGL includes
#ifndef GL_GLEXT_PROTOTYPES
#  define GL_GLEXT_PROTOTYPES
#endif
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

namespace webui {

    void setMainLoop(void (*loop)(void));
    int defaultWidth();
    int defaultHeight();

    // XHR
    class RequestXHR {
    public:
        ~RequestXHR();
        static RequestXHR query(const char* req);

    private:
        RequestXHR(char* data, int nData): data(data), nData(nData) { }

        char* data;
        int nData;
    };

}

#if defined(NANO_IOSTREAM) || !defined(__EMSCRIPTEN__)
#  include <iostream>
#else

// fake cout
namespace std {

    class ostream {
    public:
        template <typename T>
        inline ostream& operator<<(const T&) { return *this; }
    };

    extern ostream cout;
    extern char endl;

}

#endif
