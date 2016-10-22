/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "compatibility.h"
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#  include "compatibility_emscripten.cc"
#else
#  include "compatibility_desktop.cc"
#endif

namespace webui {

    // class RequestXHR common part
    RequestXHR::RequestXHR(): status(Empty), data(nullptr), nData(0) {
    }

    RequestXHR::~RequestXHR() {
        clear();
    }

    void RequestXHR::clear() {
        status = Empty;
        free(data);
        data = nullptr;
        nData = 0;
    }

    void RequestXHR::onLoadStatic(unsigned, void* ctx, void* buffer, unsigned nBuffer) {
        reinterpret_cast<RequestXHR*>(ctx)->onLoad(reinterpret_cast<char*>(buffer), nBuffer);
    }

    void RequestXHR::onErrorStatic(unsigned, void* ctx, int bytes, const char* msg) {
        reinterpret_cast<RequestXHR*>(ctx)->onError(bytes, msg);
    }

    void RequestXHR::onProgressStatic(unsigned, void* ctx, int bytes, int total) {
        reinterpret_cast<RequestXHR*>(ctx)->onProgress(bytes, total);
    }

    void RequestXHR::onLoad(char* buffer, int nBuffer) {
        data = buffer;
        nData = nBuffer;
        status = Ready;
        LOG("XHR load %d bytes", nData);
    }

    void RequestXHR::onError(int bytes, const char* msg) {
        status = Error;
        LOG("XHR error %d bytes: %s", bytes, msg);
    }

    void RequestXHR::onProgress(int bytes, int total) {
    }

}
