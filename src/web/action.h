/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>
#include "types.h"
#include "type_widget.h"
#include "compatibility.h"
#include "string_manager.h"

namespace webui {

    class Widget;
    class Actions;
    class MLParser;

    struct StackFrame {
        StackFrame() { }
        StackFrame(long l DIAG(, Type type)): l(l) DIAG(, type(type)) { }
        union {
            float f;
            long l;
            StringId strId;
            RGBA color;
            const char* text;
            void* voidPtr;
        };
        DIAG(Type type);
    };
    typedef std::vector<StackFrame> Stack;

    typedef void (*FunctionProto)();

    enum class Function: uint16_t {
        Error,
        ToggleVisible,
        BeginPath,
        Moveto,
        Lineto,
        Bezierto,
        ClosePath,
        RoundedRect,
        FillColor,
        FillVertGrad,
        Fill,
        StrokeWidth,
        StrokeColor,
        Stroke,
        Font,
        Text,
        TextCharPtr,
        TextLeft,
        TranslateCenter,
        Scale100,
        ResetTransform,
        Set,
        Query,
        Log,                         // log(StringId)
        Add,                         // push(pop + pop)
        Sub,                         // push(pop - pop)
        Mul,                         // push(pop * pop)
        Div,                         // push(pop / pop)
        Mod,                         // push(pop % pop)
        AssignFloat,                 //
        AssignColor,                 //
        AssignBit0,                  //
        AssignBit1,                  //
        AssignBit2,                  //
        AssignBit3,                  //
        AssignBit4,                  //
        AssignBit5,                  //
        AssignBit6,                  //
        AssignBit7,                  //
    };

    enum class Instruction: uint8_t {
                                     //   ins  sub   param
        Return,                      // [ ins, 0x00, 0x0000    ]
        Nop,                         // [ ins, 0x00, 0x0000    ]
        PushConstant,                // [ ins, type, elems     ] [ value]
        PushProperty,                // [ ins, type, offset/sz ]
        PushForeignProperty,         // [ ins, type, offset/sz ] [ widget* ]
        PushPropertyPtr,             // [ ins, type, offset/sz ]
        PushForeignPropertyPtr,      // [ ins, type, offset/sz ] [ widget* ]
        FunctionCall,                // [ ins, type, func      ]
    };


    struct Command {
        Command(Instruction inst, Type type = Type::Unknown, int off = 0): instruction(uint8_t(inst)), sub(int(type)), param(off) { }
        Command(Instruction inst, Type type, Function func): instruction(uint8_t(inst)), sub(int(type)), param(int(func)) { }
        Command(float f): f(f) { }
        Command(StringId strId): strId(strId) { }
        Command(RGBA color): color(color) { }
        Command(Widget* widget): widget(widget) { }
        Command(void* voidPtr): voidPtr(voidPtr) { }
        inline Instruction inst() const { return Instruction(instruction); }
        inline Type type() const { return Type(sub); }

        union {
            struct {
                uint32_t instruction:8;
                uint32_t sub:8;
                uint32_t param:16;
            };
            float f;
            long l;
            StringId strId;
            RGBA color;
            Widget* widget;
            void* voidPtr;
        };
    };


    class Actions {
    public:
        Actions();

        // returns an index for the actions or 0 in case of error
        int add(MLParser& parser, int iEntry, int fEntry);

        // execute
        template <bool DryRun = false>
        bool execute(int iAction, Widget* widget);

        // dump action and stack
        DIAG(void dump(int i) const);
        DIAG(static void dumpStack());
        DIAG(const Stack& getStack() const);

    private:
        // compiled actions
        std::vector<Command> actions;

        bool addRecur(MLParser& parser, int iEntry, int fEntry);
        bool checkFunctionParams(int iFunction, int iAction, Widget* widget);
        static long getPropertyData(const void* data, int offSz);
        static int getSizeEncoding(int size);
        const Property* resolveProperty(Command* command, Widget* widget, Widget*& propWidget);

        DIAG(const char* valueToString(Type type, const Command& action, char* buffer, int nBuffer) const);
    };

}
