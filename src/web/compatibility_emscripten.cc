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


    // class RequestXHR
    RequestXHR::~RequestXHR() {
    }

    void RequestXHR::query(const char* req) {
        emscripten_async_wget_data(req, this, onLoadStatic, onErrorStatic);
    }

}
