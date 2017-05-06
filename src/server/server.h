/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "application.h"
#include <string>
#include <cstdint>
#include <functional>

namespace server {

    class Server {
    public:
        Server();

        // add resources
        int addFont(const std::string& fontPath);

        void onCreateApp(std::function<void*(Application&)> h) { createAppHandler = h; }
        void onDestroyApp(std::function<void(void* user)> h) { destroyAppHandler = h; }

        bool run(uint16_t port, const std::string& documentRoot = ".");

    private:
        friend class Application;

        // resources
        std::string documentRoot;
        std::vector<std::string> fonts;

        // hooks
        std::function<void*(Application&)> createAppHandler;
        std::function<void(void*)> destroyAppHandler;
    };

}
