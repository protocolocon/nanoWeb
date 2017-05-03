/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "server.h"
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

namespace server {

    bool Server::run() {
        uWS::Hub hub;

        hub.onMessage([this](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
                cout << "M: {" << string(message, length) << "} " << opCode << endl;
                //ws->send(message, length, opCode);
            });

        hub.onError([](void *user) {
                cout << "error" << endl;
            });

        hub.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes) {
                auto urlHeader(req.getUrl());
                auto url(string(urlHeader.value, urlHeader.valueLength));
                if (url == "/") url = "/index.html";
                stringstream ss;
                ss << ifstream("." + url).rdbuf();
                auto content(ss.str());
                res->end(content.data(), content.size());
            });

        hub.listen(9999);
        thread background([&]() { hub.run(); });

        while (true) {
            this_thread::sleep_for(chrono::milliseconds(1000));
            hub.getDefaultGroup<uWS::SERVER>().broadcast("Ping", 4, uWS::OpCode::BINARY);
        }

        return true;
    }

}
