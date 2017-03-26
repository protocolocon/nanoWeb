/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "compatibility.h"
#include "widget.h"
#include "context.h"
#include "application.h"
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#  include "compatibility_emscripten.cc"
#else
#  include "compatibility_desktop.cc"
#endif

namespace {

    webui::Cursor currentCursor;

}

namespace webui {

    void setCursorInner(Cursor c);

    // cursors common part
    void setCursor(Cursor c) {
        if (c != currentCursor) {
            currentCursor = c;
            setCursorInner(c);
        }
    }

    // class RequestXHR common part
    RequestXHR::RequestXHR(StringId id, StringId req):
        id(id), req(req), data(nullptr), nData(0) {
        query();
    }

    char* RequestXHR::buildQuery(char* buffer, int nBuffer) {
        int iBuffer(snprintf(buffer, nBuffer, "%s?id=%s", Context::strMng.get(req), Context::strMng.get(id)));

        // get params from widget
        const auto& widgets(Context::app.getWidgets());
        auto it(widgets.find(id));
        if (it != widgets.end()) {
            char bufferParams[1024];
            const char* params(it->second->queryParams(bufferParams, sizeof(bufferParams)));
            if (params)
                snprintf(buffer + iBuffer, nBuffer - iBuffer, "&%s", params);
        }
        return buffer;
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
