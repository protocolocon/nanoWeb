/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "server.h"
#include "uWS.h"
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

namespace server {

    Server::Server():
        createAppHandler([](Application&) { return nullptr; }),
        destroyAppHandler([](void*) { }) {
    }

    bool Server::run(uint16_t port, const string& docRoot) {
        documentRoot = docRoot;
        uWS::Hub hub;

        hub.onConnection([this](uWS::WebSocket<uWS::SERVER>* ws, uWS::HttpRequest req) {
                auto* app(new Application(ws, *this));
                app->userData = createAppHandler(*app);
                ws->setUserData(app);
            });

        hub.onDisconnection([this](uWS::WebSocket<uWS::SERVER>* ws, int code, char *message, size_t length) {
                auto* app(reinterpret_cast<Application*>(ws->getUserData()));
                destroyAppHandler(app->userData);
                delete app;
            });

        hub.onError([this](void* user) {
                auto* app(reinterpret_cast<Application*>(user));
                destroyAppHandler(app->userData);
                delete app;
            });

        hub.onMessage([this](uWS::WebSocket<uWS::SERVER>* ws, char* message, size_t length, uWS::OpCode opCode) {
                auto* app(reinterpret_cast<Application*>(ws->getUserData()));
                app->pushData(message, length);
            });

        hub.onHttpRequest([this](uWS::HttpResponse* res, uWS::HttpRequest req, char* data, size_t length, size_t remainingBytes) {
                auto urlHeader(req.getUrl());
                auto url(string(urlHeader.value, urlHeader.valueLength));
                if (url == "/") url = "/index.html";
                stringstream ss;
                ss << ifstream(documentRoot + url).rdbuf();
                auto content(ss.str());
                res->end(content.data(), content.size());
                cout << "HTTP: " << url << endl;
            });

        hub.listen(port);
        thread background([&]() {
                cout << "listening on: " << port << endl;
                hub.run();
            });

        while (true) {
            this_thread::sleep_for(chrono::milliseconds(30000));
            //hub.getDefaultGroup<uWS::SERVER>().broadcast("Ping", 4, uWS::OpCode::BINARY);
        }

        return true;
    }

    int Server::addFont(const string& fontPath) {
        fonts.push_back(fontPath);
        return fonts.size() - 1;
    }

}
