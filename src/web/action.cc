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

    Type ErrorPrototype[] = { Type::LastType };
    void FunctionError() {
        DIAG(LOG("error function"));
    }

    Type LogPrototype[] = { Type::StrId, Type::LastType };
    void FunctionLog() {
        LOG("%s", Context::strMng.get(stack.back().strId));
        stack.pop_back();
    }

    Type FloatFloatPrototype[] = { Type::Float, Type::Float, Type::LastType };
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
        stack.back().f /= f;
    }

    void FunctionAssign() {
        LOG("TODO");
    }

    const struct FunctionList {
        Identifier id;
        Function funcIdx;
        FunctionProto func;
        Type* prototype;
        Type retType;
    } functionList[] = {
        { Identifier::InvalidId,       Function::Error,           FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::toggleVisible,   Function::ToggleVisible,   FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::beginPath,       Function::BeginPath,       FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::moveto,          Function::Moveto,          FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::lineto,          Function::Lineto,          FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::bezierto,        Function::Bezierto,        FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::closePath,       Function::ClosePath,       FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::roundedRect,     Function::RoundedRect,     FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::fillColor,       Function::FillColor,       FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::fillVertGrad,    Function::FillVertGrad,    FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::fill,            Function::Fill,            FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::strokeWidth,     Function::StrokeWidth,     FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::strokeColor,     Function::StrokeColor,     FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::stroke,          Function::Stroke,          FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::font,            Function::Font,            FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::text,            Function::Text,            FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::textLeft,        Function::TextLeft,        FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::translateCenter, Function::TranslateCenter, FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::scale100,        Function::Scale100,        FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::resetTransform,  Function::ResetTransform,  FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::set,             Function::Set,             FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::query,           Function::Query,           FunctionError,  ErrorPrototype,      Type::LastType },
        { Identifier::log,             Function::Log,             FunctionLog,    LogPrototype,        Type::LastType },
        { Identifier::add,             Function::Add,             FunctionAdd,    FloatFloatPrototype, Type::Float },
        { Identifier::sub,             Function::Sub,             FunctionSub,    FloatFloatPrototype, Type::Float },
        { Identifier::mul,             Function::Mul,             FunctionMul,    FloatFloatPrototype, Type::Float },
        { Identifier::div,             Function::Div,             FunctionDiv,    FloatFloatPrototype, Type::Float },
        { Identifier::mod,             Function::Mod,             FunctionMod,    ErrorPrototype,      Type::LastType },
        { Identifier::assign,          Function::Assign,          FunctionAssign, ErrorPrototype,      Type::LastType },
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
        DIAG(LOG("execute: %s", DryRun ? "dry run" : "for real"); dump(iAction); int iActionOrig(iAction));
        stack.clear();
        if (DryRun) locations.clear();
        while (true) {
            const auto& action(actions[iAction]);
            switch (action.inst()) {
            case Instruction::Return:
                DIAG(if (DryRun) { LOG("after dry run"); dump(iActionOrig); });
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
                abort();
            case Instruction::FunctionCall: {
                auto& func(functionList[action.param]);
                if (!DryRun) func.func();
                else if (!checkFunctionParams(action.param, iAction, widget)) return false;
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
                    actions.push_back(Command(Context::strMng.add(entry->pos, parser.size(iEntry))));
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
                LOG("invalid ML entry type: %s", MLParser::toString(entry->type()));
                return false;
            }
            iEntry = entry->next;
        }
        return true;
    }

#if 0
    bool Actions::pushSymbol(MLParser& parser, int& iEntry, Widget* widget) {
        bool foreign(false);
        if (parser[iEntry + 1].type() == MLParser::EntryType::Attribute) {
            // widgetId.property: get widgetId
            auto widgetId(Context::strMng.search(parser[iEntry].pos, parser.size(iEntry)));
            if (!widgets.count(widgetId)) {
                DIAG(LOG("unknown widget: %.*s", parser.size(iEntry), parser[iEntry].pos));
                return false;
            }
            widget = widgets[widgetId];
            foreign = true;
            iEntry++;
        }
        // property
        auto propId(Context::strMng.search(parser[iEntry].pos, parser.size(iEntry)));
        if (!propId.valid()) {
            DIAG(LOG("identifier '%.*s' is not registered", parser.size(iEntry), parser[iEntry].pos));
            return false;
        }
        // property
        const Property* prop(widget->getProp(Identifier(propId.getId())));
        if (prop) {
            actions.push_back(Command(foreign ? Instruction::PushForeignProperty : Instruction::PushProperty, prop->type, prop->pos));
            if (foreign)
                actions.push_back(Command(widget));
            return true;
        }
        // add constant id as it's not property
        if (widgets.count(propId) && !foreign) {
            actions.push_back(Command(propId));
            return true;
        }
        DIAG(LOG("property id '%.*s' not available on widget", parser.size(iEntry), parser[iEntry].pos));
        return false;
    }
#endif

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

    bool Actions::checkFunctionParams(int iFunction, int iAction, Widget* widget) {
        const auto& func(functionList[iFunction]);
        assert(stack.size() == locations.size());
        const Type* proto(func.prototype);
        int iStack(stack.size());
        while (*proto != Type::LastType) {
            if (--iStack < 0) {
                LOG("run out of parameters for function %s: got", Context::strMng.get(func.id));
                dumpStack();
                return false;
            }
            auto& command(actions[locations[iStack]]);
            auto type(command.type());
            if (type != *proto) {
                // try to do the execution conversion
                if (command.inst() == Instruction::PushConstant && type == Type::Id) {
                    auto& param(actions[locations[iStack] + 1]);
                    // cast push constant id to push property
                    auto propId(param.strId);
                    assert(propId.valid());
                    const Property* prop(widget->getProp(Identifier(propId.getId())));
                    if (prop) {
                        if (prop->type == *proto) {
                            // conversion ok
                            command = Command(Instruction::PushProperty, prop->type, prop->pos);
                            param = Command(Instruction::Nop);
                        } else
                            DIAG(LOG("cannot cast '%s' from %s to %s", Context::strMng.get(propId), toString(prop->type), toString(*proto)));
                    } else
                        DIAG(LOG("cannot resolve property '%s'", Context::strMng.get(propId)));
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
            stack.pop_back();
            locations.pop_back();
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
            char buffer[1024];
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
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), value(%s)", i, "Push constant",
                        ::toString(Type(actions[i].sub)),
                        ::toString(Type(actions[i].sub), &actions[i + 1], buffer, sizeof(buffer)));
                    i += 1 + actions[i].param;
                    break;
                case Instruction::PushProperty:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d)",
                        i, "Push prop", ::toString(Type(actions[i].sub)), actions[i].param);
                    break;
                case Instruction::FunctionCall:
                    LOG("%6d " GREEN "%-20s " RESET ": func(%s) -> %s",
                        i, "Function call", ::toString(Function(actions[i].param)), toString(functionList[actions[i].param].retType));
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
