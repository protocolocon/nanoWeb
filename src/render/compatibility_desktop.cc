/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "render.h"
#include "uWS.h"
#include <mutex>
#include <thread>
#include <cstring>
#include <cassert>
#include <sys/time.h>

using namespace std;
using namespace render;

namespace {

    // cursors
    GLFWcursor* cursors[int(Cursor::Last)];

    bool mainLoopRunning(true);

    // server communication
    uWS::Hub hub;
    mutex serverReadMtx;
    vector<uint8_t> serverReadData;
    bool serverRestarted;

}

namespace render {

    void setCommandLine(int argc, char** argv) {
        // get server address
        if (argc >= 2) {
            serverIp = argv[1];
            if (argc >= 3)
                serverPort = argv[2];
            LOG("using specified server: %s:%s", serverIp, serverPort);
        } else {
            LOG("no server address specified, using: %s:%s", serverIp, serverPort);
            LOG("use: %s [<domain | IP> [<port>]]", argv[0]);
        }
    }

    void setMainLoop(void (*loop)(void)) {
        // initialize cursors
        cursors[1] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
        cursors[2] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

        // server communication
        hub.onConnection([](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) {
                LOG("connected");
                serverRestarted = true;
            });
        hub.onDisconnection([](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
                LOG("disconnected");
                this_thread::sleep_for(chrono::milliseconds(100));
                hub.connect("ws://"s + serverIp + ':' + serverPort);
            });
        hub.onError([](void *user) {
                LOG("error");
                this_thread::sleep_for(chrono::milliseconds(100));
                //hub.connect("ws://"s + serverIp + ':' + serverPort);
                cancelMainLoop();
            });
        hub.onMessage([](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
                assert(opCode == uWS::OpCode::BINARY);
                lock_guard<mutex> guard(serverReadMtx);
                serverReadData.resize(serverReadData.size() + length);
                memcpy(serverReadData.data() + serverReadData.size() - length, message, length);
            });
        hub.connect("ws://"s + serverIp + ':' + serverPort);
        thread networking([]() { hub.run(); });
        this_thread::sleep_for(chrono::milliseconds(10));

        // synchronous main loop
        while (mainLoopRunning) {
            loop();
            this_thread::sleep_for(chrono::milliseconds(10));
        };

        networking.join();
    }

    void cancelMainLoop() {
        mainLoopRunning = false;
    }

    int defaultWidth() {
        return 1280;
    }

    int defaultHeight() {
        return 1024;
    }

    int getTimeNowMs() {
        struct timeval now;
        gettimeofday(&now, nullptr);
        return now.tv_sec * 1000 + now.tv_usec / 1000;
    }


    // cursors
    void setCursorInner(Cursor c) {
        assert(size_t(c) < sizeof(cursors) / sizeof(cursors[0]));
        glfwSetCursor(render.getWin(), cursors[int(c)]);
    }


    // server communication
    bool serverRefresh() {
        if (serverRestarted) return serverRestarted = false;
        return true;
    }

    int serverRead(uint8_t* data, int nData) {
        lock_guard<mutex> guard(serverReadMtx);
        nData = min(nData, int(serverReadData.size()));
        memcpy(data, serverReadData.data(), nData);
        serverReadData.erase(serverReadData.begin(), serverReadData.begin() + nData);
        return nData;
    }

    int serverWrite(const uint8_t* data, int nData) {
        hub.getDefaultGroup<uWS::CLIENT>().broadcast((const char*)data, nData, uWS::OpCode::BINARY);
        return nData;
    }

}
