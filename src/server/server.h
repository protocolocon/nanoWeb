/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "server_app.h"
#include <string>
#include <cstdint>
#include <functional>

namespace server {

    class Server {
    public:
        Server();

        void onCreateApp(std::function<void*(ServerApp&)> h) { createAppHandler = h; }
        void onDestroyApp(std::function<void(void* user)> h) { destroyAppHandler = h; }

        bool run(uint16_t port, const std::string& documentRoot = ".");

        const std::string& getDocumentRoot() const { return documentRoot; }

    private:
        // resources
        std::string documentRoot;

        // hooks
        std::function<void*(ServerApp&)> createAppHandler;
        std::function<void(void*)> destroyAppHandler;
    };

}
