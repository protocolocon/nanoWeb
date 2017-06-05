/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "client_app.h"
#include "font.h"
#include "context.h"
#include "protocol.h"
#include <cassert>
#include <cstring>
#include <cstdlib>

using namespace std;
using namespace prot;
using namespace render;

namespace {

    inline void mainIteration() {
        ctx.mainIteration();
    }

    extern "C" {
        void javascriptCanvasResize(int width, int height) {
            ctx.resize(width, height);
        }
    }

}

namespace render {

    ClientApp::ClientApp(int argc, char* argv[]) {
        assert(!app && "Only one application allowed");
        app = this;
        setCommandLine(argc, argv);
        if (!ctx.initialize(DIAG(true, true)))
            LOG("error initializing");
    }

    void ClientApp::onInit(const function<void ()>& onInit) {
        appOnInit = onInit;
    }

    void ClientApp::onConnected(const function<void ()>& onConnected) {
        appOnConnected = onConnected;
    }

    void ClientApp::onReceiveMessage(const function<void (char* message, int size)>& onReceiveMsg) {
        appOnReceiveMessage = onReceiveMsg;
    }

    void ClientApp::onReceiveResource(const function<void ()>& onReceiveResource) {
        appOnReceiveResource = onReceiveResource;
    }

    int ClientApp::fontAdd(const char* url) {
        int iFont(fonts.size());
        fonts.resize(iFont + 1);
        auto urlSize(strlen(url));
        FontResource font(iFont, urlSize);
        serverWrite((const uint8_t*)&font, sizeof(font));
        serverWrite((const uint8_t*)url, urlSize);
        return iFont;
    }

    bool ClientApp::fontCheck(int iFont) {
        return iFont >= 0 && iFont < int(fonts.size()) && fonts[iFont].initialized();
    }

    void ClientApp::text(int font, int fontSize, const char* str) {
        assert(font >= 0 && font < int(fonts.size()));
        fonts[font].text(str, str + strlen(str), fontSize, atlas, vertexBuffer);
    }

    bool ClientApp::sendMessage(const char* message, int length) {
        Chunk chunk(ChunkType::Application, length);
        return 
            serverWrite((const uint8_t*)&chunk, sizeof(chunk)) == sizeof(chunk) &&
            serverWrite((const uint8_t*)message, length) == length;
    }

    bool ClientApp::run() {
        setMainLoop(mainIteration);
        return true;
    }

}
