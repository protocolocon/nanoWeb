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
#else
#  include <cstdio>
#  define LOG(msg, ...) printf(msg "\n", ##__VA_ARGS__)
#endif

// openGL includes
#ifndef GL_GLEXT_PROTOTYPES
#  define GL_GLEXT_PROTOTYPES
#endif
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

#include "string_manager.h"

namespace webui {

    class Application;

    void setCommandLine(int argc, char** argv);
    void setMainLoop(void (*loop)(void));
    void cancelMainLoop();
    int defaultWidth();
    int defaultHeight();
    int getTimeNowMs();

    // XHR
    class RequestXHR {
    public:
        enum Type { TypeApplication, TypeTemplate, TypeFont, TypeLast };

        RequestXHR(Application& app, Type type, StringId id, const char* req);
        ~RequestXHR();
        void query(const char* req);
        void makeCString();

        // getters
        inline char* getData() { return data; }
        inline const char* getData() const { return data; }
        inline const int getNData() const { return nData; }
        inline char operator[](int i) const { return data[i]; }
        inline Type getType() const { return type; }
        inline StringId getId() const { return id; }

    private:
        static void onLoadStatic(void* ctx, void* buffer, int nBuffer);
        static void onErrorStatic(void* ctx);
        static size_t onAddDataStatic(char* data, size_t size, size_t nmemb, RequestXHR* xhr);
        void onLoad(char* buffer, int nBuffer);
        void onError();
        size_t onAddData(char* data, size_t size, size_t nmemb);

        Application& app;
        Type type;
        StringId id;
        char* data;
        int nData;
    };

}
