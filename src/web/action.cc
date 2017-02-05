/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "action.h"
#include "widget.h"
#include "context.h"
#include "ml_parser.h"

using namespace std;
using namespace webui;

namespace {

    // to reduce code size, make context global
    Stack stack;

    DIAG(void checkStack(int n, const vector<Type>& types) {
            if (int(stack.size()) < n)
                LOG("expected %d parameters at least, got %zu", n, stack.size());
            else {
                for (int i = stack.size() - n, j = 0; i < int(stack.size()); i++, j++) {
                    if (stack[i].type != types[j])
                        LOG("expected parameter %d of type %s, got %s", j, toString(types[j]), toString(stack[i].type));
                }
            }
        });

    void FunctionError() {
        DIAG(LOG("error function"));
    }

    void FunctionLog() {
        DIAG(checkStack(1, { Type::StrId }));
        LOG("%s", Context::strMng.get(stack.back().strId));
        stack.pop_back();
    }

    void FunctionAdd() {
        DIAG(checkStack(2, { Type::Float, Type::Float }));
        float f(stack.back().f);
        stack.pop_back();
        stack.back().f += f;
    }

    void FunctionSub() {
        DIAG(checkStack(2, { Type::Float, Type::Float }));
        float f(stack.back().f);
        stack.pop_back();
        stack.back().f -= f;
    }

    void FunctionMul() {
        DIAG(checkStack(2, { Type::Float, Type::Float }));
        float f(stack.back().f);
        stack.pop_back();
        stack.back().f *= f;
    }

    void FunctionDiv() {
        DIAG(checkStack(2, { Type::Float, Type::Float }));
        float f(stack.back().f);
        stack.pop_back();
        stack.back().f /= f;
    }

    void FunctionMod() {
        DIAG(checkStack(2, { Type::Color, Type::Float }));
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
    } functionList[] = {
        { Identifier::InvalidId,       Function::Error,           FunctionError },
        { Identifier::toggleVisible,   Function::ToggleVisible,   FunctionError },
        { Identifier::beginPath,       Function::BeginPath,       FunctionError },
        { Identifier::moveto,          Function::Moveto,          FunctionError },
        { Identifier::lineto,          Function::Lineto,          FunctionError },
        { Identifier::bezierto,        Function::Bezierto,        FunctionError },
        { Identifier::closePath,       Function::ClosePath,       FunctionError },
        { Identifier::roundedRect,     Function::RoundedRect,     FunctionError },
        { Identifier::fillColor,       Function::FillColor,       FunctionError },
        { Identifier::fillVertGrad,    Function::FillVertGrad,    FunctionError },
        { Identifier::fill,            Function::Fill,            FunctionError },
        { Identifier::strokeWidth,     Function::StrokeWidth,     FunctionError },
        { Identifier::strokeColor,     Function::StrokeColor,     FunctionError },
        { Identifier::stroke,          Function::Stroke,          FunctionError },
        { Identifier::font,            Function::Font,            FunctionError },
        { Identifier::text,            Function::Text,            FunctionError },
        { Identifier::textLeft,        Function::TextLeft,        FunctionError },
        { Identifier::translateCenter, Function::TranslateCenter, FunctionError },
        { Identifier::scale100,        Function::Scale100,        FunctionError },
        { Identifier::resetTransform,  Function::ResetTransform,  FunctionError },
        { Identifier::set,             Function::Set,             FunctionError },
        { Identifier::query,           Function::Query,           FunctionError },
        { Identifier::log,             Function::Log,             FunctionLog },
        { Identifier::add,             Function::Add,             FunctionAdd },
        { Identifier::sub,             Function::Sub,             FunctionSub },
        { Identifier::mul,             Function::Mul,             FunctionMul },
        { Identifier::div,             Function::Div,             FunctionDiv },
        { Identifier::mod,             Function::Mod,             FunctionMod },
        { Identifier::assign,          Function::Assign,          FunctionAssign },
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

    int Actions::add(MLParser& parser, int iEntry, int fEntry, Widget* widget) {
        int dev(actions.size());
        if (parser[iEntry].type() == MLParser::EntryType::List) iEntry++; // list
        if (!addRecur(parser, iEntry, fEntry, widget)) return 0;
        actions.push_back(Command(Instruction::Return));
        return dev;
    }

    bool Actions::execute(int iAction, Widget* widget) {
        stack.clear();
        while (true) {
            const auto& action(actions[iAction]);
            switch (action.inst()) {
            case Instruction::Return:
                return true;
            case Instruction::PushConstant:
                stack.push_back(StackFrame(actions[iAction + 1].l DIAG(, action.type())));
                ++iAction;
                break;
            case Instruction::PushProperty:
                stack.push_back(StackFrame(getPropertyData(widget, action.param) DIAG(, action.type())));
                break;
            case Instruction::PushForeignProperty:
                abort();
            case Instruction::FunctionCall:
                functionList[action.param].func();
                break;
            default:
                DIAG(LOG("unknown instruction: %d", action.instruction));
                abort();
            }
            ++iAction;
        }
        return true;
    }

    bool Actions::addRecur(MLParser& parser, int iEntry, int fEntry, Widget* widget) {
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
                if (!addRecur(parser, iEntry + 1, entry->next, widget)) return false;
                else {
                    auto func(funcById(Context::strMng.search(entry->pos, parser.size(iEntry)).getId()));
                    if (func == Function::Error) {
                        DIAG(LOG("unknown function name: %.*s", parser.size(iEntry), entry->pos));
                        return false;
                    }
                    actions.push_back(Command(Instruction::FunctionCall, func));
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
                case Instruction::PushConstant:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), value(%s)", i, "Push constant",
                        ::toString(Type(actions[i].sub)),
                        ::toString(Type(actions[i].sub), &actions[i + 1], buffer, sizeof(buffer)));
                    i++;
                    break;
                case Instruction::PushProperty:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d)",
                        i, "Push prop", ::toString(Type(actions[i].sub)), actions[i].param);
                    break;
                case Instruction::FunctionCall:
                    LOG("%6d " GREEN "%-20s " RESET ": func(%s)",
                        i, "Function call", ::toString(Function(actions[i].param)));
                    break;
                default:
                    LOG("%6d " RED "%-20s" RESET, i, "internal error");
                    break;
                }
                i++;
            }
        });

    DIAG(void Actions::dumpStack() const {
            LOG("stack: %zu", stack.size());
            char buffer[128];
            for (int i = 0; i < int(stack.size()); i++)
                LOG("%4d " GREEN "%20s " RESET "%s",
                    i, toString(stack[i].type), toString(stack[i].type, &stack[i].l, buffer, sizeof(buffer)));
        });

}
