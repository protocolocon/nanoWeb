/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <cstddef>
#include <emscripten.h>

using namespace std;

namespace webui {

    void setMainLoop(void (*loop)(void)) {
        emscripten_set_main_loop(loop, 0 /* fps */, true /* infinite loop*/);
    }

    int defaultWidth() {
        return EM_ASM_INT_V(return canvas.width);
    }

    int defaultHeight() {
        return EM_ASM_INT_V(return canvas.height);
    }


    // class RequestXHR
    RequestXHR::~RequestXHR() {
    }

    RequestXHR RequestXHR::query(const char* req) {
        void* buffer;
        int nBuffer, error;
        emscripten_wget_data(req, &buffer, &nBuffer, &error);
        if (error)  {
            cout << "error in query: " << req << endl;
            return RequestXHR(nullptr, 0);
        }
        return RequestXHR(reinterpret_cast<char*>(buffer), nBuffer);
    }

}

// fake cout
#if !defined(NANO_IOSTREAM)
namespace std {
    ostream cout;
    char endl;
}
#endif
