/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <vector>
#include <cstdint>

namespace prot {

    struct ApplicationBase {
        uint32_t transferProgress;
        std::vector<uint8_t> transferBuffer;

        inline ApplicationBase();
        inline void reset();
        inline void expectTransfer(int size);
        inline void continueTransfer(int size);
    };

    enum class Command: uint32_t {
        Text,
        Font,
    };

    struct ProtoBase {
        ProtoBase(Command com, uint32_t size): com(com), size(size) { }
        Command com;
        uint32_t size;
    };

    struct Text: ProtoBase {
        Text(int font, int fontSize, float x, float y, int strSize):
            ProtoBase(Command::Text, sizeof(Text) - sizeof(ProtoBase) + strSize),
            font(font), fontSize(fontSize), x(x), y(y) { }
        int font;
        int fontSize;
        float x;
        float y;
        char str[];
    };

    struct FontResource: ProtoBase {
        FontResource(int font, int dataSize):
            ProtoBase(Command::Font, sizeof(FontResource) - sizeof(ProtoBase) + dataSize),
            font(font) { }
        int font;
        uint8_t data[];
    };


    // Application base implementation
    ApplicationBase::ApplicationBase() {
        transferBuffer.reserve(1 << 16);
        reset();
    }

    void ApplicationBase::reset() {
        expectTransfer(sizeof(ProtoBase));
    }

    void ApplicationBase::expectTransfer(int size) {
        transferProgress = 0;
        transferBuffer.resize(size);
    }

    void ApplicationBase::continueTransfer(int size) {
        transferBuffer.resize(size);
    }

}
