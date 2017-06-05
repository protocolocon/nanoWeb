/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "communication.h"
#include "font.h"
#include "context.h"
#include <cassert>

using namespace prot;

namespace render {

    void ChunkedCommunication::refresh() {
        if (!serverRefresh()) {
            // server context is lost, will reconnect and create a new session
            reset();
        }

        // read cycle from network
        int read;
        assert(transferProgress < transferBuffer.size());
        while ((read = serverRead(&transferBuffer[transferProgress], transferBuffer.size() - transferProgress))) {
            transferProgress += read;
            if (transferProgress == transferBuffer.size()) {
                // completed transfer
                Chunk* chunk(reinterpret_cast<Chunk*>(transferBuffer.data()));
                if (transferProgress == sizeof(Chunk)) continueTransfer(sizeof(Chunk) + chunk->size);
                if (transferProgress == transferBuffer.size()) {
                    switch (chunk->type) {
                    case ChunkType::Application:
                        appOnReceiveMessage((char*)transferBuffer.data() + sizeof(Chunk), transferBuffer.size() - sizeof(Chunk));
                        break;
                    case ChunkType::Session:
                        if (!initialized) { initialized = true; appOnInit(); }
                        appOnConnected();
                        break;
                    case ChunkType::FontResource:
                        fontResource();
                        break;
                    }
                    reset();
                }
            }
        }
    }

    void ChunkedCommunication::fontResource() {
        FontResource* font(reinterpret_cast<FontResource*>(transferBuffer.data()));
        assert(font->font < int(fonts.size()));
        fonts[font->font].init((uint8_t*)font->data, font->getSize());
        LOG("font received: %d %d bytes", font->font, font->getSize());
        appOnReceiveResource();
    }

}
