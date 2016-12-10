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

#define PROPDIFF(ptr1, ptr0) \
    { { uint32_t(int(Type::Int32) | 4 << 8 | 0 << 12 | 0 << 15 | uint16_t((ptr1) - (ptr0)) << 16) } }

namespace webui {

    class Application;

    enum class Type: uint8_t {
        Bit,
        Uint8,
        Int16,
        Int32,
        Id,
        Str,
        StrId,           // removes quotes if any
        Float,
        Color,
        ColorModif,      // color modificaiton or color
        SizeRelative,
        Coord,           // with ref
        FontIdx,
        ActionTable,
        Text,            // char*
        TextPropOrStrId, // TextPropOrStrId type
        LastType
    };

    const char* toString(Type t);

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
        long get(Identifier id, const void* data) const;
        void set(Identifier id, void* data, long value) const;
    };

}
