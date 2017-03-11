/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <cassert>
#include <cstddef>

using namespace std;

namespace webui {

    void setCommandLine(int argc, char** argv) {
    }

    void setMainLoop(void (*loop)(void)) {
        emscripten_set_main_loop(loop, 0 /* fps */, true /* infinite loop*/);
    }

    void cancelMainLoop() {
        emscripten_cancel_main_loop();
    }

    int defaultWidth() {
        return EM_ASM_INT_V(return canvas.width);
    }

    int defaultHeight() {
        return EM_ASM_INT_V(return canvas.height);
    }

    void log(const char* format, ...) {
    }

    int getTimeNowMs() {
        return int(emscripten_get_now());
    }


    // cursors
    void setCursorInner(Cursor c) {
        switch (c) {
        case Cursor::Default: EM_ASM(Module.canvas.style.cursor = 'default'); break;
        case Cursor::Pointer: EM_ASM(Module.canvas.style.cursor = 'pointer'); break;
        default: DIAG(LOG("invalid cursor"));
        }
    }


    // class RequestXHR
    RequestXHR::RequestXHR(StringId id, StringId req, const char* data, int nData):
        id(id), req(req), data(nullptr), nData(0) {
    }

    RequestXHR::~RequestXHR() {
    }

    void RequestXHR::query() {
        char buffer[1024];
        emscripten_async_wget_data(buildQuery(buffer, sizeof(buffer)), this, onLoadStatic, onErrorStatic);
    }

}
