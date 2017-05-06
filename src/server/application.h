/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "uWS.h"
#include "protocol.h"
#include <string>

namespace server {

    class Server;

    class Application: protected prot::ApplicationBase {
    public:
        Application();
        Application(uWS::WebSocket<uWS::SERVER>* ws, Server& server);

        void pushData(char* message, size_t length);

        void text(int font, int fontSize, const std::string& str, float x, float y);

    private:
        friend class Server;
        void* userData;
        uWS::WebSocket<uWS::SERVER>* ws;
        Server* server;

        void commandFont();
    };

}
