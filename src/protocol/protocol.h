/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <vector>
#include <cstdint>

namespace prot {

    struct ChunkedCommunicationBase {
        uint32_t transferProgress;
        std::vector<uint8_t> transferBuffer;

        inline ChunkedCommunicationBase();
        inline void reset();
        inline void expectTransfer(int size);
        inline void continueTransfer(int size);
    };

    enum class ChunkType: uint32_t {
        Application,
        Session,
        FontResource,
    };

    struct Chunk {
        Chunk(ChunkType type, uint32_t size): type(type), size(size) { }
        ChunkType type;
        uint32_t size; // of next chunk
    };


    // Out of band messages
    struct FontResource: Chunk {
        FontResource(int font, uint32_t size): Chunk(ChunkType::FontResource, sizeof(FontResource) - sizeof(Chunk) + size), font(font) { }
        uint32_t getSize() const { return size - sizeof(FontResource) + sizeof(Chunk); }
        int font;
        char data[];
    };


    // Chunked communication base base implementation
    ChunkedCommunicationBase::ChunkedCommunicationBase() {
        transferBuffer.reserve(1 << 16);
        reset();
    }

    void ChunkedCommunicationBase::reset() {
        expectTransfer(sizeof(Chunk));
    }

    void ChunkedCommunicationBase::expectTransfer(int size) {
        transferProgress = 0;
        transferBuffer.resize(size);
    }

    void ChunkedCommunicationBase::continueTransfer(int size) {
        transferBuffer.resize(size);
    }

}
