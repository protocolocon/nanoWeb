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

#include "string_manager.h"

namespace webui {

    void setCommandLine(int argc, char** argv);
    void setMainLoop(void (*loop)(void));
    void cancelMainLoop();
    int defaultWidth();
    int defaultHeight();
    int getTimeNowMs();

    // cursors
    enum class Cursor {
        Default,
        Pointer,
        Last
    };
    void setCursor(Cursor c);

    // XHR
    class RequestXHR {
    public:
        RequestXHR(StringId id, StringId req);
        RequestXHR(StringId id, StringId req, const char* data, int nData);
        ~RequestXHR();
        void query();
        void makeCString();

        // getters
        inline char* getData() { return data; }
        inline const char* getData() const { return data; }
        inline const int getNData() const { return nData; }
        inline char operator[](int i) const { return data[i]; }
        inline StringId getId() const { return id; }
        inline StringId getReq() const { return req; }

    private:
        char* buildQuery(char* buffer, int nBuffer);
        static void onLoadStatic(void* ctx, void* buffer, int nBuffer);
        static void onErrorStatic(void* ctx);
        static size_t onAddDataStatic(char* data, size_t size, size_t nmemb, RequestXHR* xhr);
        void onLoad(char* buffer, int nBuffer);
        void onError();
        size_t onAddData(char* data, size_t size, size_t nmemb);

        StringId id;
        StringId req;
        char* data;
        int nData;
    };

}
