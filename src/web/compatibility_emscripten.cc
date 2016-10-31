/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <cassert>
#include <cstddef>

using namespace std;

namespace webui {

    void setMainLoop(int argc, char** argv, void (*loop)(void)) {
        emscripten_set_main_loop(loop, 0 /* fps */, true /* infinite loop*/);
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
    void RequestXHR::query(const char* req, const char* param) {
        assert(status != Pending && "internal: XHR query in a pending request");
        clear();
        status = Pending;
        emscripten_async_wget2_data(req, "GET", param, this, false, onLoadStatic, onErrorStatic, onProgressStatic);
    }

}
