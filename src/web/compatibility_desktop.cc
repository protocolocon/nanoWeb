/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <thread>

using namespace std;

namespace webui {

    void setMainLoop(void (*loop)(void)) {
        while (true) {
            loop();
            this_thread::sleep_for(chrono::milliseconds(10));
        };
    }

    int defaultWidth() {
        return 1280;
    }

    int defaultHeight() {
        return 1024;
    }

    // class RequestXHR
    RequestXHR::~RequestXHR() {
    }

    RequestXHR RequestXHR::query(const char* req) {
        return RequestXHR(nullptr, 0);
    }

}
