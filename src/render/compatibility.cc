/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "compatibility.h"
#include "context.h"
#include <stdlib.h>

namespace {

    render::Cursor currentCursor;

    // server communication
    const char* serverIp = "127.0.0.1";
    const char* serverPort = "3000";

}

#ifdef __EMSCRIPTEN__
#  include "compatibility_emscripten.cc"
#else
#  include "compatibility_desktop.cc"
#endif

namespace render {

    void setCursorInner(Cursor c);

    // cursors common part
    void setCursor(Cursor c) {
        if (c != currentCursor) {
            currentCursor = c;
            setCursorInner(c);
        }
    }

    // server communication
    void serverInit(const char* ip, const char* port) {
        serverIp = ip;
        serverPort = port;
    }

}
