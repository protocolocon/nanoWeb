/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "properties.h"
#include "string_manager.h"

namespace webui {

    const char* toString(Type t) {
        const char* strs[int(Type::LastType)] = {
            "Uint8",
            "Int16",
            "Int32",
            "StrId",
            "Float",
            "Color",
            "SizeRelative",
            "ActionTable",
            "ActionEntry",
        };
        return strs[int(t)];
    }

    int Properties::get(Identifier id, const void* data) const {
        auto it(find(id));
        if (it == end()) {
            LOG("unknown property in get");
            return 0;
        }
        const auto& prop(it->second);
        switch (prop.size) {
        case 0: return unsigned((reinterpret_cast<const uint8_t *>(data)[prop.pos] >> prop.bit) & 1);
        case 1: return unsigned( reinterpret_cast<const uint8_t *>(data)[prop.pos]);
        case 2: return           reinterpret_cast<const  int16_t*>(data)[prop.pos];
        case 4: return           reinterpret_cast<const  int16_t*>(data)[prop.pos];
        default: LOG("internal error"); return 0;
        }
    }

    void Properties::set(Identifier id, void* data, int value) const {
        auto it(find(id));
        if (it == end()) {
            LOG("unknown property in set");
            return;
        }
        const auto& prop(it->second);
        switch (prop.size) {
        case 0: {
                uint8_t& v(reinterpret_cast<uint8_t *>(data)[prop.pos]);
                uint8_t mask(1 << prop.bit);
                v &= ~mask;
                v |= (value << prop.bit) & mask;
            }
            break;
        case 1: reinterpret_cast<uint8_t *>(data)[prop.pos] = uint8_t(value); break;
        case 2: reinterpret_cast< int16_t*>(data)[prop.pos] = int16_t(value); break;
        case 4: reinterpret_cast< int16_t*>(data)[prop.pos] =         value;  break;
        default: LOG("internal error");
        }
    }

}
