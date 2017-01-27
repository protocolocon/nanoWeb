/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>
#include "compatibility.h"
#include "string_manager.h"

namespace webui {

    class MLParser;
    class Widget;

    enum class TypeBase: uint8_t {
        Float,
        Id,                          // index to string manager
        String,                      // const char*
        Color,
    };

    enum class Instruction: uint8_t {
        Return,                      // [ ins, 0x00, 0x0000 ]
        PushFloatConstant,           // [ ins, 0x00, 0x0000 ] [ float]
        PushProperty,                // [ ins, type, offset ]
        PushForeignProperty,         // [ ins, type, offset ] [ widget* ]
    };


    struct Command {
        Command(Instruction inst, uint8_t sub = 0, uint16_t param = 0): instruction(uint8_t(inst)), sub(sub), param(param) { }
        Command(float f): f(f) { }
        inline Instruction inst() const { return Instruction(instruction); }

        union {
            struct {
                uint32_t instruction:8;
                uint32_t sub:8;
                uint32_t param:16;
            };
            float f;
        };
    };


    class Actions {
    public:
        using WidgetMap = std::unordered_map<StringId, Widget*, StringId>;

        Actions(StringManager& strMng, WidgetMap& widgets);

        // returns an index for the actions or 0 in case of error
        int add(MLParser& parser, int iEntry, int fEntry, Widget* widget);

        // dump action
        DIAG(void dump(int i) const);

    private:
        // compiled actions
        std::vector<Command> actions;
        StringManager& strMng;
        WidgetMap& widgets;

        bool pushSymbol(MLParser& parser, int iEntry, Widget* widget);
    };

}
