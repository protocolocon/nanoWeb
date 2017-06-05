/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include <functional>

namespace render {

    class ClientApp {
    public:
        ClientApp(int argc, char* argv[]);

        // callbacks
        void onInit(const std::function<void ()>& onInit);
        void onConnected(const std::function<void ()>& onConnected);
        void onReceiveMessage(const std::function<void (char* message, int size)>& onReceiveMsg);
        void onReceiveResource(const std::function<void ()>& onReceiveResource);

        // fonts
        int fontAdd(const char* url);
        bool fontCheck(int iFont);

        // application
        void text(int font, int fontSize, const char* str);

        // communication with server
        bool sendMessage(const char* message, int length);

        // main loop
        bool run();
    };

}
