/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "server_app.h"
#include "server.h"
#include <cassert>
#include <sstream>
#include <fstream>
#include <iostream>


using namespace std;
using namespace prot;

namespace server {

    ServerApp::ServerApp(): userData(nullptr), ws(nullptr), server(nullptr), onReceiveMsg([](char* message, int size) { }) {
    }

    ServerApp::ServerApp(uWS::WebSocket<uWS::SERVER>* ws, Server& server): userData(nullptr), ws(ws), server(&server) {
    }

    void ServerApp::onReceiveMessage(const std::function<void (char* message, int size)>& onReceiveMsg_) {
        onReceiveMsg = onReceiveMsg_;
    }

    void ServerApp::sendMessage(const char* message, size_t length) {
        Chunk chunk(ChunkType::Application, length);
        ws->send((const char*)&chunk, sizeof(chunk), uWS::OpCode::BINARY);
        ws->send(message, length, uWS::OpCode::BINARY);
    }

    void ServerApp::pushData(char* message, size_t length) {
        int lengthProgress(0);
        assert(transferProgress < transferBuffer.size());
        while (length) {
            auto toRead(min(transferBuffer.size() - transferProgress, length));
            memcpy(&transferBuffer[transferProgress], message + lengthProgress, toRead);
            transferProgress += toRead;
            length -= toRead;
            lengthProgress += toRead;
            if (transferProgress == transferBuffer.size()) {
                // completed transfer
                Chunk* chunk(reinterpret_cast<Chunk*>(transferBuffer.data()));
                if (transferProgress == sizeof(Chunk)) continueTransfer(sizeof(Chunk) + chunk->size);
                if (transferProgress == transferBuffer.size()) {
                    switch (chunk->type) {
                    case ChunkType::Application:
                        onReceiveMsg((char*)transferBuffer.data() + sizeof(Chunk), transferBuffer.size() - sizeof(Chunk));
                        break;
                    case ChunkType::Session: abort();
                    case ChunkType::FontResource:
                        fontResource();
                        break;
                    }
                    reset();
                }
            }
        }
    }

    void ServerApp::fontResource() {
        FontResource* font(reinterpret_cast<FontResource*>(transferBuffer.data()));
        cout << "Server font: " << font->font << ' ' << string(font->data, font->getSize()) << endl;
        // send font to client
        stringstream ss;
        ss << ifstream(server->getDocumentRoot() + '/' + string(font->data, font->getSize())).rdbuf();
        auto content(ss.str());
        font->size = sizeof(FontResource) - sizeof(Chunk) + content.size();
        ws->send((char*)font, sizeof(FontResource), uWS::OpCode::BINARY);
        ws->send(content.data(), content.size(), uWS::OpCode::BINARY);
    }
}
