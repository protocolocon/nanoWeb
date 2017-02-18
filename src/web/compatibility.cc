/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "compatibility.h"
#include "context.h"
#include "application.h"
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#  include "compatibility_emscripten.cc"
#else
#  include "compatibility_desktop.cc"
#endif

namespace webui {

    // class RequestXHR common part
    RequestXHR::RequestXHR(StringId id, StringId req):
        id(id), req(req), data(nullptr), nData(0) {
        query();
    }

    void RequestXHR::onLoadStatic(void* ctx, void* buffer, int nBuffer) {
        reinterpret_cast<RequestXHR*>(ctx)->onLoad(reinterpret_cast<char*>(buffer), nBuffer);
    }

    void RequestXHR::onErrorStatic(void* ctx) {
        reinterpret_cast<RequestXHR*>(ctx)->onError();
    }

    size_t RequestXHR::onAddDataStatic(char* data, size_t size, size_t nmemb, RequestXHR* xhr) {
        return xhr->onAddData(data, size, nmemb);
    }

    void RequestXHR::onLoad(char* buffer, int nBuffer) {
        data = buffer;
        nData = nBuffer;
        DIAG(LOG("XHR load %d bytes", nData));
        Context::app.onLoad(this);
    }

    void RequestXHR::onError() {
        LOG("XHR error");
        Context::app.onError(this);
    }

    void RequestXHR::makeCString() {
        // add zero at the end (required by JSON)
        data = (char*)realloc(data, nData + 1);
        data[nData] = 0;
    }

}
