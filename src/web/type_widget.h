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
    uint32_t(int(Type::type) | size << 8 | bit << 12 | redundant << 15 | uint16_t(long(&((class*)nullptr)->member) / size) << 16)

namespace webui {

    class Application;
    class StringManager;

    enum class Type: uint8_t {
        Unknown,
        Bit,
        Uint8,
        Int16,
        Int32,
        Id,
        Str,
        StrId,           // removes quotes if any
        Float,
        Color,
        ColorModif,      // color modification (reference + '%') or color constant
        SizeRelative,    // holds a constant that can be relative (%) or absolute; adaptative
        Coord,           // with ref and optional '+' / '-' operator
        FontIdx,
        ActionTable,
        Text,            // char*
        TextPropOrStrId, // TextPropOrStrId type: a Text or a StrId
        LastType
    };

    DIAG(const char* toString(Type t));

    struct Property {
        Property(): all(0) { }
        Property(uint32_t all): all(all) { }
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

    class TypeWidget: public std::unordered_map<Identifier, Property> {
    public:
        using Map = std::unordered_map<Identifier, Property>;

        TypeWidget(Identifier type, int size, const Map& properties): Map(properties), type(type), size(size) { }

        Identifier type;
        int size;

        // generic interface
        long get(Identifier id, const void* data) const;
        void set(Identifier id, void* data, long value) const;

        DIAG(void dump(const StringManager& strMng, int indent) const);
    };

}
