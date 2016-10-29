/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

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

namespace webui {

    void setMainLoop(int argc, char** argv, void (*loop)(void));
    int defaultWidth();
    int defaultHeight();

    // XHR
    class RequestXHR {
    public:
        RequestXHR();
        ~RequestXHR();
        void query(const char* req, const char* param = "");
        void makeCString();
        void clear();

        // status
        enum Status { Empty, Pending, Ready, Error };

        // getters
        inline Status getStatus() const { return status; }
        inline char* getData() { return data; }
        inline const char* getData() const { return data; }
        inline const int getNData() const { return nData; }
        inline char operator[](int i) const { return data[i]; }

    private:
        static void onLoadStatic(unsigned, void* ctx, void* buffer, unsigned nBuffer);
        static void onErrorStatic(unsigned, void* ctx, int bytes, const char* msg);
        static void onProgressStatic(unsigned, void* ctx, int bytes, int total);
        static size_t onAddDataStatic(char* data, size_t size, size_t nmemb, RequestXHR* xhr);
        void onLoad(char* buffer, int nBuffer);
        void onError(int bytes, const char* msg);
        void onProgress(int bytes, int total);
        size_t onAddData(char* data, size_t size, size_t nmemb);

        Status status;
        char* data;
        int nData;
    };

}
