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
        StackFrame(long l): l(l) { }
        union {
            float f;
            long l;
            uint32_t u32;
            StringId strId;
            RGBA color;
            const char* text;
            void* voidPtr;
        };
    };
    typedef std::vector<StackFrame> Stack;

    typedef void (*FunctionProto)();

    enum class Function: uint16_t {
        Error,
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
        TextLeftCharPtr,
        TextWidth,
        TextWidthCharPtr,
        Translate,
        Scale,
        ResetTransform,
        Scissor,
        ResetScissor,
        Query,
        TriggerTimers,
        Log,                         // log(StringId)
        Add,                         // push(pop + pop)
        Sub,                         // push(pop - pop)
        Mul,                         // push(pop * pop)
        Div,                         // push(pop / pop)
        Mod,                         // push(pop % pop)
        AssignUint32,                //
        AssignSizeRel,               //
        AssignUint8,                 //
        AssignInt16,                 //
        AssignInt32,                 //
        AssignText,                  //
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
        Return,                      // [ ins, 0x00, 0x0000 ]
        Nop,                         // [ ins, 0x00, 0x0000 ]
        PushConstant,                // [ ins, type, elems  ] [ value]
        PushProperty,                // [ ins, type, offset ]
        PushForeignProperty,         // [ ins, type, offset ] [ widget* ]
        PushDoubleProperty,          // [ ins, type, offset ] [ variable prop position ]
        PushParentProperty,          // [ ins, type, offset ] [ ancestor level ]
        PushDoubleParentProperty,    // [ ins, type, offset ] [ variable prop position ] [ ancestor level ]
        PushPropertyPtr,             // [ ins, type, offset ]
        PushForeignPropertyPtr,      // [ ins, type, offset ] [ widget* ]
        PushDoublePropertyPtr,       // [ ins, type, offset ] [ variable prop position ]
        PushParentPropertyPtr,       // [ ins, type, offset ] [ ancestor level ]
        PushDoubleParentPropertyPtr, // [ ins, type, offset ] [ variable prop position ] [ ancestor level ]
        FunctionCall,                // [ ins, type, func   ]
    };

    // property resolution types
    enum DispatchType {
        DispatchNormal       = 0,    // property                              param: none
        DispatchForeign      = 1,    // widget.property                       param: foreign widget
        DispatchDouble       = 2,    // (variable->widget).property           param: variable position
        DispatchParent       = 3,    // property (found in ancestor)          param: ancestor
        DispatchDoubleParent = 4,    // (ancestor:variable->widget).property  param: variable position | ancestor << 20
        DispatchUnknown
    };


    struct Command {
        Command() { }
        inline Command(Instruction inst, Type type = Type::Unknown, int off = 0):
            instruction(uint8_t(inst)), sub(int(type)), param(off) { DIAG(zero(sizeof(uint32_t))); }
        inline Command(Instruction inst, Type type, Function func):
            instruction(uint8_t(inst)), sub(int(type)), param(int(func)) { DIAG(zero(sizeof(uint32_t))); }
        inline Command(float f): f(f) { DIAG(zero(sizeof(float))); }
        inline Command(long l): l(l) { }
        inline Command(StringId strId): strId(strId) { DIAG(zero(sizeof(StringId))); }
        inline Command(RGBA color): color(color) { DIAG(zero(sizeof(RGBA))); }
        inline Command(Widget* widget): widget(widget) { }
        inline Command(const char* text): text(text) { }
        inline Command(void* voidPtr): voidPtr(voidPtr) { }
        inline Instruction inst() const { return Instruction(instruction); }
        inline Type type() const { return Type(sub); }

        DIAG(void zero(int s) { memset((char*)this + s, 0, sizeof(Command) - s); });

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
            const char* text;
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
        bool execute(int iAction, Widget* widget); // false on error
        bool executeOrEmpty(int iAction, Widget* widget) { return iAction ? execute(iAction, widget) : false; } // false on error or empty

        // evaluate property: executes an action and sets corresponding value to property
        bool evalProperty(MLParser& parser, int iEntry, int fEntry, StringId propId, Widget* widget, bool onlyIfTemplated, bool define);

        // dump action and stack
        DIAG(void dump(int i) const);
        DIAG(static void dumpStack());
        DIAG(const Stack& getStack() const);

    private:
        // compiled actions
        std::vector<Command> actions;
        bool templateFound;
        bool defining;

        bool addRecur(MLParser& parser, int iEntry, int fEntry);
        bool checkFunctionParams(int iFunction, int iAction, Widget* widget);
        static long getPropertyData(const void* data, Command command);
        const Property* resolveProperty(Command* command, Widget* widget, DispatchType& type, long& param);
        void resolvePropertyRecode(const Property* prop, DispatchType type, long param, Command* command, bool ptr);
        static Widget* resolveDoubleDispatch(int pos, Widget* widget);

        DIAG(const char* valueToString(Type type, const Command& action, char* buffer, int nBuffer) const);
    };

}
