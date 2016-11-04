/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include "reserved_words.h"
#include <unordered_map>

#define PROP(class, member, type, size, bit, redundant) \
    { { uint32_t(int(Type::type) | size << 8 | bit << 12 | redundant << 15 | uint16_t(long(&((class*)nullptr)->member) / size) << 16) } }

namespace webui {

    class Application;

    enum class Type: uint8_t {
        Uint8,
        Int16,
        Int32,
        StrId,
        Float,
        Color,
        SizeRelative,
        Action,
    };

    struct Property {
        union {
            uint32_t all;
            struct {
                Type type;
                uint8_t size:4;
                uint8_t bit:3;       // position of bit if a bitset
                uint8_t redundant:1; // if other field also sets this data
                uint16_t pos;        // multiple of the size
            };
        };
    };

    class Properties: public std::unordered_map<Identifier, Property> {
    public:
        using std::unordered_map<Identifier, Property>::unordered_map;

        // generic interface
        int get(Identifier id, const void* data) const;
        void set(Identifier id, void* data, int value) const;
    };

}
