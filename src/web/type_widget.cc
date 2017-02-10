/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "type_widget.h"
#include "context.h"
#include "string_manager.h"

namespace webui {

    DIAG(const char* toString(Type t) {
            const char* strs[int(Type::LastType) + 1] = {
                "Unknown",
                "Bit",
                "Uint8",
                "Int16",
                "Int32",
                "Id",
                "Str",
                "StrId",
                "Float",
                "Color",
                "ColorModif",
                "SizeRelative",
                "Coord",
                "FontIdx",
                "ActionTable",
                "Text",
                "TextPropOrStrId",
                "VoidPtr",
                "Void", // last
            };
            return strs[int(t)];
        });

    DIAG(const char* toString(Type t, const void* data, char* buffer, int nBuffer) {
            switch (t) {
            case Type::Float: snprintf(buffer, nBuffer, "%.4f", *reinterpret_cast<const float*>(data)); break;
            case Type::Id: snprintf(buffer, nBuffer, "%s", Context::strMng.get(*reinterpret_cast<const StringId*>(data))); break;
            case Type::StrId: snprintf(buffer, nBuffer, "\"%s\"", Context::strMng.get(*reinterpret_cast<const StringId*>(data))); break;
            case Type::FontIdx: snprintf(buffer, nBuffer, "%ld", *reinterpret_cast<const long*>(data)); break;
            case Type::Color: snprintf(buffer, nBuffer, "%08x", *reinterpret_cast<const uint32_t*>(data)); break;
            case Type::ActionTable: snprintf(buffer, nBuffer, "%d", *reinterpret_cast<const uint32_t*>(data)); break;
            case Type::Text: snprintf(buffer, nBuffer, "\"%s\"", *reinterpret_cast<char* const*>(data)); break;
            case Type::VoidPtr: snprintf(buffer, nBuffer, "%p", *reinterpret_cast<void* const*>(data)); break;
            default: snprintf(buffer, nBuffer, RED "value error" RESET);
            }
            return buffer;
        });

    long TypeWidget::get(Identifier id, const void* data) const {
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
        case 4: return           reinterpret_cast<const  int32_t*>(data)[prop.pos];
        case 8: return           reinterpret_cast<const  int64_t*>(data)[prop.pos];
        default: LOG("internal error"); return 0;
        }
    }

    void TypeWidget::set(Identifier id, void* data, long value) const {
        auto it(find(id));
        if (it == end()) {
            LOG("unknown property in set");
            return;
        }
        const auto& prop(it->second);
        if (prop.type == Type::Text) {
            reinterpret_cast<char**>(data)[prop.pos] = value ? strdup(*reinterpret_cast<char**>(&value)) : nullptr;
        } else {
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
            case 4: reinterpret_cast< int32_t*>(data)[prop.pos] = int32_t(value); break;
            case 8: reinterpret_cast< int64_t*>(data)[prop.pos] =         value;  break;
            default: LOG("internal error");
            }
        }
    }

    DIAG(void TypeWidget::dump(int indent, const void* widget) const {
            char buffer[1024];
            for (const auto& idProp: *this) {
                const auto& prop(idProp.second);
                LOG("%*s%-20s: %-16s %4d %2d  " GREEN "%s" RESET,
                    indent, "", Context::strMng.get(idProp.first), toString(prop.type), prop.pos * prop.size, prop.size,
                    toString(prop.type, (uint8_t*)widget + prop.pos * prop.size, buffer, sizeof(buffer)));
            }
        });

}
