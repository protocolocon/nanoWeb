/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "uWS.h"
#include "protocol.h"
#include <vector>
#include <string>

namespace server {

    class Server;

    class ServerApp: protected prot::ChunkedCommunicationBase {
    public:
        // async interface
        void onReceiveMessage(const std::function<void (char* message, int size)>& onReceiveMsg);

        void sendMessage(const char* message, size_t length);

    private:
        friend class Server;
        void* userData;
        uWS::WebSocket<uWS::SERVER>* ws;
        Server* server;

        // async
        std::function<void (char* message, int size)> onReceiveMsg;

        ServerApp();
        ServerApp(uWS::WebSocket<uWS::SERVER>* ws, Server& server);
        void pushData(char* message, size_t length);
        void fontResource();
    };

}
