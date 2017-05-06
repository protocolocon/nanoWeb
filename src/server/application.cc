/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "server.h"
#include <cassert>
#include <sstream>
#include <fstream>
#include <iostream>


using namespace std;
using namespace prot;

namespace server {

    Application::Application(): userData(nullptr), ws(nullptr), server(nullptr) {
    }

    Application::Application(uWS::WebSocket<uWS::SERVER>* ws, Server& server): userData(nullptr), ws(ws), server(&server) {
    }

    void Application::pushData(char* message, size_t length) {
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
                ProtoBase* base(reinterpret_cast<ProtoBase*>(transferBuffer.data()));
                if (transferProgress == sizeof(ProtoBase)) continueTransfer(sizeof(ProtoBase) + base->size);
                if (transferProgress == transferBuffer.size()) {
                    switch (base->com) {
                    case Command::Text: assert(false); break;
                    case Command::Font: commandFont(); break;
                    }
                    reset();
                }
            }
        }
    }

    void Application::text(int font, int fontSize, const std::string& str, float x, float y) {
        Text text(font, fontSize, x, y, int(str.size()));
        ws->send((char*)&text, sizeof(text), uWS::OpCode::BINARY);
        ws->send(str.data(), str.size(), uWS::OpCode::BINARY);
    }

    void Application::commandFont() {
        FontResource* font(reinterpret_cast<FontResource*>(transferBuffer.data()));
        cout << "Font: " << font->font << endl;
        if (font->font >= int(server->fonts.size())) {
            cout << "Request for invalid font" << endl;
            assert(false);
            return;
        }
        // send font to client
        stringstream ss;
        ss << ifstream(server->documentRoot + '/' + server->fonts[font->font]).rdbuf();
        auto content(ss.str());
        font->size = sizeof(FontResource) - sizeof(ProtoBase) + content.size();
        ws->send((char*)font, sizeof(FontResource), uWS::OpCode::BINARY);
        ws->send(content.data(), content.size(), uWS::OpCode::BINARY);
    }

}
