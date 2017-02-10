/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "action.h"
#include "widget.h"
#include "context.h"
#include "ml_parser.h"
#include <cassert>

using namespace std;
using namespace webui;

namespace {

    // to reduce code size, make context global
    Stack stack;
    vector<int> locations;
    Widget* execWidget;

    Type VoidPrototype[] = { Type::LastType };
    Type FloatPrototype[] = { Type::Float, Type::LastType };
    Type Float2Prototype[] = { Type::Float, Type::Float, Type::LastType };
    Type Float5Prototype[] = { Type::Float, Type::Float, Type::Float, Type::Float, Type::Float, Type::LastType };
    Type Float6Prototype[] = { Type::Float, Type::Float, Type::Float, Type::Float, Type::Float, Type::Float, Type::LastType };
    Type ColorPrototype[] = { Type::Color, Type::LastType };
    Type StrIdPrototype[] = { Type::StrId, Type::LastType };
    Type IdPrototype[] = { Type::Id, Type::LastType };

    Type ModPrototype[] = { Type::Float, Type::Color, Type::LastType };
    Type FillVertGradPrototype[] = { Type::Color, Type::Color, Type::Float, Type::Float, Type::LastType };
    Type FontPrototype[] = { Type::Float, Type::FontIdx, Type::LastType };
    Type TextPrototype[] = { Type::StrId, Type::Float, Type::Float, Type::LastType };
    Type TextCharPtrPrototype[] = { Type::Text, Type::Float, Type::Float, Type::LastType };
    Type AssignPrototype[] = { Type::Unknown, Type::VoidPtr, Type::LastType };

    void FunctionError() {
        DIAG(LOG("error function"));
    }

    void FunctionToggleVisible() {
        assert(stack.size() >= 1);
        Context::app.executeToggleVisible(stack.back().strId);
        stack.pop_back();
    }

    void FunctionBeginPath() {
        Context::render.beginPath();
    }

    void FunctionMoveto() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        Context::render.moveto(s[0].f, s[1].f);
        stack.resize(stack.size() - 2);
    }

    void FunctionLineto() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        Context::render.lineto(s[0].f, s[1].f);
        stack.resize(stack.size() - 2);
    }

    void FunctionBezierto() {
        assert(stack.size() >= 6);
        auto* s(&stack.back() - 5);
        Context::render.bezierto(s[0].f, s[1].f, s[2].f, s[3].f, s[4].f, s[5].f);
        stack.resize(stack.size() - 6);
    }

    void FunctionClosePath() {
        Context::render.beginPath();
    }

    void FunctionStrokeWidth() {
        assert(stack.size() >= 1);
        Context::render.strokeWidth(stack.back().f);
        stack.pop_back();
    }

    void FunctionStrokeColor() {
        assert(stack.size() >= 1);
        Context::render.strokeColor(stack.back().color);
        stack.pop_back();
    }

    void FunctionStroke() {
        Context::render.stroke();
    }

    void FunctionFont() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        Context::render.font(s[0].l);
        Context::render.fontSize(s[1].f);
        stack.resize(stack.size() - 2);
    }

    inline void FunctionTextCommon(float x, float y, const char* text) {
        V2f pos(x, y);
        pos += execWidget->box.pos + execWidget->box.size * 0.5f;
        Context::render.textAlign(NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        Context::render.text(pos.x, pos.y, text);
    }

    void FunctionText() {
        assert(stack.size() >= 3);
        auto* s(&stack.back() - 2);
        FunctionTextCommon(s[0].f, s[1].f, Context::strMng.get(s[2].l));
        stack.resize(stack.size() - 3);
    }

    void FunctionTextCharPtr() {
        assert(stack.size() >= 3);
        auto* s(&stack.back() - 2);
        FunctionTextCommon(s[0].f, s[1].f, s[2].text);
        stack.resize(stack.size() - 3);
    }

    void FunctionRoundedRect() {
        assert(stack.size() >= 5);
        auto* s(&stack.back() - 4);
        Context::render.roundedRect(s[0].f, s[1].f, s[2].f, s[3].f, s[4].f);
        stack.resize(stack.size() - 5);
    }

    void FunctionFillColor() {
        assert(stack.size() >= 1);
        Context::render.fillColor(stack.back().color);
        stack.pop_back();
    }

    void FunctionFillVertGrad() {
        assert(stack.size() >= 4);
        auto* s(&stack.back() - 3);
        Context::render.fillVertGrad(s[0].f, s[1].f, s[2].color, s[3].color);
        stack.resize(stack.size() - 4);
    }

    void FunctionFill() {
        Context::render.fill();
    }

    void FunctionLog() {
        LOG("%s", Context::strMng.get(stack.back().strId));
        stack.pop_back();
    }

    void FunctionAdd() {
        float f(stack.back().f);
        stack.pop_back();
        stack.back().f += f;
    }

    void FunctionSub() {
        float f(stack.back().f);
        stack.pop_back();
        stack.back().f -= f;
    }

    void FunctionMul() {
        float f(stack.back().f);
        stack.pop_back();
        stack.back().f *= f;
    }

    void FunctionDiv() {
        float f(stack.back().f);
        stack.pop_back();
        stack.back().f /= f;
    }

    void FunctionMod() {
        float f(stack.back().f);
        stack.pop_back();
        stack.back().color.multRGB(f * 2.56f);
    }

    void FunctionAssignFloat() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        *reinterpret_cast<float*>(s[0].voidPtr) = s[1].f;
        stack.resize(stack.size() - 2);
    }

    void FunctionAssignColor() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        *reinterpret_cast<RGBA*>(s[0].voidPtr) = s[1].color;
        stack.resize(stack.size() - 2);
    }

    template <int bit>
    void FunctionAssignBit() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        if (s[1].f > 0.5f)
            *reinterpret_cast<uint8_t*>(s[0].voidPtr) |= 1 << bit;
        else
            *reinterpret_cast<uint8_t*>(s[0].voidPtr) &= ~(1 << bit);
        stack.resize(stack.size() - 2);
    }

    const struct FunctionList {
        Identifier id;
        Function funcIdx;
        FunctionProto func;
        Type* prototype;
        Type retType;
    } functionList[] = {
        { Identifier::InvalidId,       Function::Error,           FunctionError,        VoidPrototype,         Type::LastType },
        { Identifier::toggleVisible,   Function::ToggleVisible,   FunctionToggleVisible,IdPrototype,           Type::LastType },
        { Identifier::beginPath,       Function::BeginPath,       FunctionBeginPath,    VoidPrototype,         Type::LastType },
        { Identifier::moveto,          Function::Moveto,          FunctionMoveto,       Float2Prototype,       Type::LastType },
        { Identifier::lineto,          Function::Lineto,          FunctionLineto,       Float2Prototype,       Type::LastType },
        { Identifier::bezierto,        Function::Bezierto,        FunctionBezierto,     Float6Prototype,       Type::LastType },
        { Identifier::closePath,       Function::ClosePath,       FunctionClosePath,    VoidPrototype,         Type::LastType },
        { Identifier::roundedRect,     Function::RoundedRect,     FunctionRoundedRect,  Float5Prototype,       Type::LastType },
        { Identifier::fillColor,       Function::FillColor,       FunctionFillColor,    ColorPrototype,        Type::LastType },
        { Identifier::fillVertGrad,    Function::FillVertGrad,    FunctionFillVertGrad, FillVertGradPrototype, Type::LastType },
        { Identifier::fill,            Function::Fill,            FunctionFill,         VoidPrototype,         Type::LastType },
        { Identifier::strokeWidth,     Function::StrokeWidth,     FunctionStrokeWidth,  FloatPrototype,        Type::LastType },
        { Identifier::strokeColor,     Function::StrokeColor,     FunctionStrokeColor,  ColorPrototype,        Type::LastType },
        { Identifier::stroke,          Function::Stroke,          FunctionStroke,       VoidPrototype,         Type::LastType },
        { Identifier::font,            Function::Font,            FunctionFont,         FontPrototype,         Type::LastType },
        { Identifier::text,            Function::Text,            FunctionText,         TextPrototype,         Type::LastType },
        { Identifier::text,            Function::TextCharPtr,     FunctionTextCharPtr,  TextCharPtrPrototype,  Type::LastType },
        { Identifier::textLeft,        Function::TextLeft,        FunctionError,        VoidPrototype,         Type::LastType },
        { Identifier::translateCenter, Function::TranslateCenter, FunctionError,        VoidPrototype,         Type::LastType },
        { Identifier::scale100,        Function::Scale100,        FunctionError,        VoidPrototype,         Type::LastType },
        { Identifier::resetTransform,  Function::ResetTransform,  FunctionError,        VoidPrototype,         Type::LastType },
        { Identifier::set,             Function::Set,             FunctionError,        VoidPrototype,         Type::LastType },
        { Identifier::query,           Function::Query,           FunctionError,        VoidPrototype,         Type::LastType },
        { Identifier::log,             Function::Log,             FunctionLog,          StrIdPrototype,        Type::LastType },
        { Identifier::add,             Function::Add,             FunctionAdd,          Float2Prototype,       Type::Float },
        { Identifier::sub,             Function::Sub,             FunctionSub,          Float2Prototype,       Type::Float },
        { Identifier::mul,             Function::Mul,             FunctionMul,          Float2Prototype,       Type::Float },
        { Identifier::div,             Function::Div,             FunctionDiv,          Float2Prototype,       Type::Float },
        { Identifier::mod,             Function::Mod,             FunctionMod,          ModPrototype,          Type::Color },
        { Identifier::assign,          Function::AssignFloat,     FunctionAssignFloat,  AssignPrototype,       Type::LastType },
        { Identifier::assign,          Function::AssignColor,     FunctionAssignColor,  AssignPrototype,       Type::LastType },
        { Identifier::assign,          Function::AssignBit0,      FunctionAssignBit<0>, AssignPrototype,       Type::LastType },
        { Identifier::assign,          Function::AssignBit1,      FunctionAssignBit<1>, AssignPrototype,       Type::LastType },
        { Identifier::assign,          Function::AssignBit2,      FunctionAssignBit<2>, AssignPrototype,       Type::LastType },
        { Identifier::assign,          Function::AssignBit3,      FunctionAssignBit<3>, AssignPrototype,       Type::LastType },
        { Identifier::assign,          Function::AssignBit4,      FunctionAssignBit<4>, AssignPrototype,       Type::LastType },
        { Identifier::assign,          Function::AssignBit5,      FunctionAssignBit<5>, AssignPrototype,       Type::LastType },
        { Identifier::assign,          Function::AssignBit6,      FunctionAssignBit<6>, AssignPrototype,       Type::LastType },
        { Identifier::assign,          Function::AssignBit7,      FunctionAssignBit<7>, AssignPrototype,       Type::LastType },
    };

    DIAG(const char* toString(Function func) {
            return Context::strMng.get(functionList[int(func)].id);
        });

    inline Function funcById(Identifier id) {
        // TODO: do binary search
        for (const auto& f: functionList)
            if (f.id == id) return f.funcIdx;
        return Function::Error;
    }
}

namespace webui {

    Actions::Actions():
        actions(1, Command(Instruction::Return)) {
    }

    int Actions::add(MLParser& parser, int iEntry, int fEntry) {
        int dev(actions.size());
        if (parser[iEntry].type() == MLParser::EntryType::List) iEntry++; // list
        if (!addRecur(parser, iEntry, fEntry)) return 0;
        actions.push_back(Command(Instruction::Return));
        return dev;
    }

    template <bool DryRun>
    bool Actions::execute(int iAction, Widget* widget) {
        if (!iAction) return true;
        DIAG(int iActionOrig(iAction));
        //DIAG(LOG("execute: %s", DryRun ? "dry run" : "for real"); dump(iAction));
        stack.clear();
        execWidget = widget;
        if (DryRun) locations.clear();
        while (true) {
            const auto& action(actions[iAction]);
            switch (action.inst()) {
            case Instruction::Return:
                //DIAG(if (DryRun) { LOG("after dry run"); dump(iActionOrig); });
                return true;
            case Instruction::Nop:
                break;
            case Instruction::PushConstant:
                stack.push_back(StackFrame(actions[iAction + 1].l DIAG(, action.type())));
                if (DryRun) locations.push_back(iAction);
                if (action.param) {
                    stack.push_back(StackFrame(actions[iAction + 2].l DIAG(, action.type())));
                    if (DryRun) locations.push_back(iAction);
                    ++iAction;
                }
                ++iAction;
                break;
            case Instruction::PushProperty:
                stack.push_back(StackFrame(getPropertyData(widget, action.param) DIAG(, action.type())));
                if (DryRun) locations.push_back(iAction);
                break;
            case Instruction::PushForeignProperty:
                stack.push_back(StackFrame(getPropertyData(actions[iAction + 1].widget, action.param) DIAG(, action.type())));
                if (DryRun) locations.push_back(iAction);
                ++iAction;
                break;
            case Instruction::PushPropertyPtr:
                stack.push_back(StackFrame(long(widget) + action.param DIAG(, action.type())));
                if (DryRun) locations.push_back(iAction);
                break;
            case Instruction::PushForeignPropertyPtr:
                stack.push_back(StackFrame(actions[iAction + 1].l + action.param DIAG(, action.type())));
                if (DryRun) locations.push_back(iAction);
                ++iAction;
                break;
            case Instruction::FunctionCall: {
                auto& func(functionList[action.param]);
                if (!DryRun) func.func();
                else if (!checkFunctionParams(action.param, iAction, widget)) {
                    DIAG(LOG("failed dry run"); dump(iActionOrig));
                    return false;
                }
                break;
            }
            default:
                DIAG(LOG("unknown instruction: %d", action.instruction));
                abort();
            }
            ++iAction;
        }
    }

    bool Actions::addRecur(MLParser& parser, int iEntry, int fEntry) {
        bool attr;
        while (iEntry < fEntry) {
            // function, expression or assignment expected
            const auto* entry(&parser[iEntry]);
            switch (entry->type()) {
            case MLParser::EntryType::Id:
                // cannot resolve the id (widget/property) at this point
                attr = (parser[iEntry + 1].type() == MLParser::EntryType::Attribute);
                actions.push_back(Command(Instruction::PushConstant, Type::Id, attr));
                actions.push_back(Command(Context::strMng.add(entry->pos, parser.size(iEntry))));
                if (attr) {
                    entry = &parser[iEntry + 1];
                    actions.push_back(Command(Context::strMng.add(entry->pos, parser.size(iEntry + 1))));
                }
                break;
            case MLParser::EntryType::Number:
                // just push it
                actions.push_back(Command(Instruction::PushConstant, Type::Float));
                actions.push_back(Command(atof(entry->pos)));
                break;
            case MLParser::EntryType::Operator:
            case MLParser::EntryType::Function:
                if (!addRecur(parser, iEntry + 1, entry->next)) return false;
                else {
                    auto func(funcById(Context::strMng.search(entry->pos, parser.size(iEntry)).getId()));
                    if (func == Function::Error) {
                        DIAG(LOG("unknown function name: %.*s", parser.size(iEntry), entry->pos));
                        return false;
                    }
                    actions.push_back(Command(Instruction::FunctionCall, functionList[int(func)].retType, func));
                }
                break;
            case MLParser::EntryType::Color:
                // just push it
                actions.push_back(Command(Instruction::PushConstant, Type::Color));
                actions.push_back(Command(RGBA(entry->pos)));
                break;
            case MLParser::EntryType::String:
                // just push it
                actions.push_back(Command(Instruction::PushConstant, Type::StrId));
                actions.push_back(Command(Context::strMng.add(entry->pos + 1, parser.size(iEntry) - 2)));
                break;
            default:
                DIAG(LOG("invalid ML entry type: %s", MLParser::toString(entry->type())));
                return false;
            }
            iEntry = entry->next;
        }
        return true;
    }

    const Property* Actions::resolveProperty(Command* command, Widget* widget, Widget*& propWidget) {
        StringId propId;
        if (command->param) {
            // x.y => widget.propery
            auto widgetId(command[1].strId);
            assert(widgetId.valid());
            if (!Context::app.getWidgets().count(widgetId)) {
                DIAG(LOG("unknown widget: %s", Context::strMng.get(widgetId)));
                return nullptr;
            }
            propWidget = Context::app.getWidgets()[widgetId];
            propId = command[2].strId;
        } else {
            propWidget = widget;
            propId = command[1].strId;
        }
        assert(propId.valid());
        // property
        const Property* prop(propWidget->getProp(Identifier(propId.getId())));
        DIAG(if (!prop) LOG("property id '%s' not available on widget", Context::strMng.get(propId)));
        return prop;
    }

    long Actions::getPropertyData(const void* data, int offSz) {
        int size(offSz & 3);
        offSz >>= 2;
        switch (size) {
        case 0: return unsigned( reinterpret_cast<const uint8_t *>(data)[offSz]);
        case 1: return           reinterpret_cast<const  int16_t*>(data)[offSz];
        case 2: return           reinterpret_cast<const  int32_t*>(data)[offSz];
        case 3: return           reinterpret_cast<const  int64_t*>(data)[offSz];
        default: LOG("internal error"); return 0;
        }
    }

    int Actions::getSizeEncoding(int size) {
        switch (size) {
        case 0:
        case 1: return 0;
        case 2: return 1;
        case 4: return 2;
        case 8: return 3;
        default: LOG("internal error"); return 0;
        }
    }

    bool Actions::checkFunctionParams(int iFunction, int iAction, Widget* widget) {
        const auto& func(functionList[iFunction]);
        assert(stack.size() == locations.size());
        const Type* proto(func.prototype);
        int iStack(stack.size() - 1);
        while (*proto != Type::LastType) {
            if (iStack < 0) {
                DIAG(
                    LOG("run out of parameters for function %s: got", Context::strMng.get(func.id));
                    dumpStack());
                return false;
            }
            auto* command(&actions[locations[iStack]]);
            auto type(command->type());
            if (type != *proto) {
                // try to do the execution conversion
                if (*proto == Type::VoidPtr) {
                    // assign or inline expressions
                    if (command->inst() != Instruction::PushConstant) {
                        DIAG(LOG("cannot assign to target at %d", locations[iStack]));
                        return false;
                    }
                    Widget* propWidget;
                    const Property* prop(resolveProperty(command, widget, propWidget));
                    if (!prop) return false;
                    // upgrade value types
                    auto valueType(actions[locations[iStack + 1]].type());
                    if (valueType == Type::Float && prop->type == Type::Bit) valueType = prop->type;
                    if (prop->type == valueType) {
                        if (command->param) {
                            command[0] = Command(Instruction::PushForeignPropertyPtr, Type::VoidPtr, prop->pos * prop->size);
                            command[1] = Command(propWidget);
                            command[2] = Command(Instruction::Nop);
                        } else {
                            command[0] = Command(Instruction::PushPropertyPtr, Type::VoidPtr, prop->pos * prop->size);
                            command[1] = Command(Instruction::Nop);
                        }
                        switch (prop->type) {
                        case Type::Float: actions[iAction] = Command(Instruction::FunctionCall, Type::Float, Function::AssignFloat); break;
                        case Type::Color: actions[iAction] = Command(Instruction::FunctionCall, Type::Color, Function::AssignColor); break;
                        case Type::Bit:   actions[iAction] = Command(Instruction::FunctionCall, Type::Bit,   Function(int(Function::AssignBit0) + prop->bit)); break;
                        default: DIAG(LOG("unknown assignment")); return false;
                        }
                    } else {
                        DIAG(LOG("cannot cast '%s' from %s to %s in %d",
                                 Context::strMng.get(command[1].strId), toString(valueType), toString(prop->type), iAction));
                        return false;
                    }
                } else if (command->inst() == Instruction::PushConstant && type == Type::Id) {
                    // cast push constant id to push property
                    Widget* propWidget;
                    const Property* prop(resolveProperty(command, widget, propWidget));
                    if (!prop) return false;
                    bool cast(true);
                    if (prop->type != *proto && *proto != Type::Unknown) {
                        // property type does not match prototype, try by changing prototype
                        if (iFunction == int(Function::Text) && prop->type == Type::Text) {
                            // change function from text to textCharPtr
                            actions[iAction].param = int(Function::TextCharPtr);
                        } else
                            // no polymorphism found
                            cast = false;
                    }

                    if (cast) {
                        // conversion of type ok
                        if (command->param) {
                            command[0] = Command(Instruction::PushForeignProperty, prop->type, prop->pos << 2 | getSizeEncoding(prop->size));
                            command[1] = Command(propWidget);
                            command[2] = Command(Instruction::Nop);
                        } else {
                            command[0] = Command(Instruction::PushProperty, prop->type, prop->pos << 2 | getSizeEncoding(prop->size));
                            command[1] = Command(Instruction::Nop);
                        }
                    } else {
                        DIAG(LOG("cannot cast '%s' from %s to %s in %d",
                                 Context::strMng.get(command[1].strId), toString(prop->type), toString(*proto), iAction));
                        return false;
                    }
                } else if (command->inst() == Instruction::PushConstant && type == Type::StrId && *proto == Type::FontIdx) {
                    // cast font name to font index
                    command->sub = int(Type::FontIdx);
                    command[1].l = Context::app.getFont(command[1].strId);
                } else if (*proto == Type::Unknown) {
                } else {
                    // cannot cast
                    DIAG(
                        LOG("expected parameter %lu of type %s, got %s in %s at position %d",
                            stack.size() - iStack, toString(*proto), toString(type), Context::strMng.get(func.id), locations[iStack]);
                        dumpStack());
                    return false;
                }
            }
            ++proto;
            auto lastLocation(locations.back());
            do {
                stack.pop_back();
                locations.pop_back();
                iStack--;
            } while (!locations.empty() && locations.back() == lastLocation);
        }
        if (func.retType != Type::LastType) {
            stack.push_back(StackFrame(0.0f DIAG(, actions[iAction].type())));
            locations.push_back(iAction);
        }
        return true;
    }

    DIAG(const Stack& Actions::getStack() const {
            return stack;
        });

    DIAG(void Actions::dump(int i) const {
            LOG("actions entry: %d", i);
            char buffer[512];
            char buffer2[512];
            while (true) {
                Command com(actions[i]);
                switch (com.inst()) {
                case Instruction::Return:
                    LOG("%6d " GREEN "%-20s" RESET, i, "Return");
                    return;
                case Instruction::Nop:
                    LOG("%6d " GREEN "%-20s" RESET, i, "Nop");
                    break;
                case Instruction::PushConstant:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), value(%s) elems(%s)", i, "Push constant",
                        ::toString(actions[i].type()),
                        ::toString(actions[i].type(), &actions[i + 1], buffer, sizeof(buffer)),
                        actions[i].param ? ::toString(actions[i].type(), &actions[i + 2], buffer2, sizeof(buffer2)) : "");
                    i += 1 + actions[i].param;
                    break;
                case Instruction::PushProperty:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d), size(%d)",
                        i, "Push prop", ::toString(actions[i].type()), actions[i].param >> 2, 1 << (actions[i].param & 3));
                    break;
                case Instruction::PushForeignProperty:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d), size(%d), widget(%p)",
                        i, "Push foreign prop", ::toString(actions[i].type()), actions[i].param >> 2, 1 << (actions[i].param & 3),
                        actions[i+1].widget);
                    i++;
                    break;
                case Instruction::PushPropertyPtr:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d)",
                        i, "Push prop ptr", ::toString(actions[i].type()), actions[i].param);
                    break;
                case Instruction::PushForeignPropertyPtr:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d), widget(%p)",
                        i, "Push foreign prop ptr", ::toString(actions[i].type()), actions[i].param,
                        actions[i+1].widget);
                    i++;
                    break;
                case Instruction::FunctionCall:
                    LOG("%6d " GREEN "%-20s " RESET ": func(" CYAN "%s" RESET ") -> %s   type(%s)",
                        i, "Function call", ::toString(Function(actions[i].param)), toString(functionList[actions[i].param].retType),
                        toString(com.type()));
                    break;
                default:
                    LOG("%6d " RED "%-20s" RESET, i, "internal error");
                    break;
                }
                i++;
            }
        });

    DIAG(void Actions::dumpStack() {
            LOG("stack: %zu", stack.size());
            char buffer[128];
            for (int i = 0; i < int(stack.size()); i++)
                LOG("%4d " GREEN "%20s " RESET "%s",
                    i, toString(stack[i].type), toString(stack[i].type, &stack[i].l, buffer, sizeof(buffer)));
        });

    // explicit template instantiations
    template bool Actions::execute<true>(int iAction, Widget* widget);
    template bool Actions::execute<false>(int iAction, Widget* widget);

}
