/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

// diagnostics
#ifdef DISABLE_DIAGNOSTICS
#  define DIAG(x, ...)
#else
#  define DIAG(x, ...)       x, ##__VA_ARGS__
#endif

// logging
#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#  define LOG(msg, ...) emscripten_log(EM_LOG_CONSOLE, msg, ##__VA_ARGS__)
#  define RED
#  define GREEN
#  define BROWN
#  define BLUE
#  define MAGENTA
#  define CYAN
#  define RESET
#else
#  include <cstdio>
#  define LOG(msg, ...) printf(msg "\n", ##__VA_ARGS__)
#  define RED       "\033[31m"
#  define GREEN     "\033[32m"
#  define BROWN     "\033[33m"
#  define BLUE      "\033[34m"
#  define MAGENTA   "\033[35m"
#  define CYAN      "\033[36m"
#  define RESET     "\033[0m"
#endif

// openGL includes
#ifndef GL_GLEXT_PROTOTYPES
#  define GL_GLEXT_PROTOTYPES
#endif
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

namespace render {

    void setCommandLine(int argc, char** argv);
    void setMainLoop(void (*loop)(void));
    void cancelMainLoop();
    int defaultWidth();
    int defaultHeight();
    int getTimeNowMs();

    // server communication
    void serverInit(const char* ip, const char* port);
    bool serverRefresh(); // returns false if a reconnection was required
    int serverRead(uint8_t* data, int nData);
    int serverWrite(const uint8_t* data, int nData);

    // cursors
    enum class Cursor {
        Default,
        Pointer,       // for links
        Hand,          // for scrolls
        Last
    };
    void setCursor(Cursor c);

}
