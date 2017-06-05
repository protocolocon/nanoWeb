/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <cerrno>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

namespace {

    int serverSock = -1;

}

namespace render {

    void setCommandLine(int argc, char** argv) {
    }

    void setMainLoop(void (*loop)(void)) {
        emscripten_set_main_loop(loop, 0 /* fps */, true /* infinite loop*/);
    }

    void cancelMainLoop() {
        emscripten_cancel_main_loop();
    }

    int defaultWidth() {
        return EM_ASM_INT_V(return canvas.width);
    }

    int defaultHeight() {
        return EM_ASM_INT_V(return canvas.height);
    }

    void log(const char* format, ...) {
    }

    int getTimeNowMs() {
        return int(emscripten_get_now());
    }


    // cursors
    void setCursorInner(Cursor c) {
        switch (c) {
        case Cursor::Default: EM_ASM(Module.canvas.style.cursor = 'default'); break;
        case Cursor::Pointer: EM_ASM(Module.canvas.style.cursor = 'pointer'); break;
        case Cursor::Hand:    EM_ASM(Module.canvas.style.cursor = 'all-scroll'); break;
        default: DIAG(LOG("invalid cursor"));
        }
    }

    // server communication
    bool serverRefresh() {
        if (serverSock < 0) {
            // reconnect
            // set server address
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(atoi(serverPort));
            inet_pton(AF_INET, serverIp, &addr.sin_addr);

            // connect with server
            if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
                LOG("cannot open socket");
            } else if (fcntl(serverSock, F_SETFL, O_NONBLOCK) < 0 ||
                (connect(serverSock, (struct sockaddr *)&addr, sizeof(addr)) < 0 && errno != EINPROGRESS)) {
                close(serverSock);
                serverSock = -1;
                LOG("cannot connect non-blocking socket");
            }
            return false;
        }
        return true;
    }

    int serverRead(uint8_t* data, int nData) {
        assert(data && nData);
        int n(recv(serverSock, data, nData, 0));
        if (n <= 0) {
            if (errno == EAGAIN) return 0; // not ready yet
            close(serverSock);
            serverSock = -1; // error, restart (next refresh will return false)
            return 0;
        }
        return n;
    }

    int serverWrite(const uint8_t* data, int nData) {
        assert(data && nData);
        int n(send(serverSock, data, nData, 0));
        if (n <= 0) {
            if (errno == EAGAIN) return 0; // not ready yet
            close(serverSock);
            serverSock = -1; // error, restart (next refresh will return false)
            return 0;
        }
        return n;
    }

}
