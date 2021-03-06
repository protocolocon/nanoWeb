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

    Type VoidPrototype[] =         { Type::LastType };
    Type FloatPrototype[] =        { Type::Float,   Type::LastType };
    Type Float2Prototype[] =       { Type::Float,   Type::Float,    Type::LastType };
    Type Float4Prototype[] =       { Type::Float,   Type::Float,    Type::Float, Type::Float, Type::LastType };
    Type Float5Prototype[] =       { Type::Float,   Type::Float,    Type::Float, Type::Float, Type::Float, Type::LastType };
    Type Float6Prototype[] =       { Type::Float,   Type::Float,    Type::Float, Type::Float, Type::Float, Type::Float, Type::LastType };
    Type ColorPrototype[] =        { Type::Color,   Type::LastType };
    Type StrIdPrototype[] =        { Type::StrId,   Type::LastType };
    Type TextPrototype[] =         { Type::Text,    Type::LastType };

    Type ModPrototype[] =          { Type::Float,   Type::Color,    Type::LastType };
    Type FillVertGradPrototype[] = { Type::Color,   Type::Color,    Type::Float, Type::Float, Type::LastType };
    Type FontPrototype[] =         { Type::Float,   Type::FontIdx,  Type::LastType };
    Type TextFuncPrototype[] =     { Type::StrId,   Type::Float,    Type::Float, Type::LastType };
    Type TextCharPtrPrototype[] =  { Type::Text,    Type::Float,    Type::Float, Type::LastType };
    Type QueryPrototype[] =        { Type::Id,      Type::StrId,    Type::LastType };
    Type AssignPrototype[] =       { Type::Unknown, Type::VoidPtr,  Type::LastType };

    void FunctionError() {
        DIAG(LOG("error function"));
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

    inline float FunctionTextCommon(float x, float y, const char* text) {
        V2f pos(x, y);
        pos += execWidget->box.pos + execWidget->box.size * 0.5f;
        Context::render.textAlign(NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        return Context::render.text(pos.x, pos.y, text);
    }

    void FunctionText() {
        assert(stack.size() >= 3);
        auto* s(&stack.back() - 2);
        float end(FunctionTextCommon(s[0].f, s[1].f, Context::strMng.get(s[2].l)));
        stack.resize(stack.size() - 2);
        stack.back().f = end;
    }

    void FunctionTextCharPtr() {
        assert(stack.size() >= 3);
        auto* s(&stack.back() - 2);
        float end(FunctionTextCommon(s[0].f, s[1].f, s[2].text));
        stack.resize(stack.size() - 2);
        stack.back().f = end;
    }

    inline float FunctionTextLeftCommon(float x, float y, const char* text) {
        V2f pos(x, y);
        pos += execWidget->box.pos;
        pos.y += execWidget->box.size.y * 0.5f;
        Context::render.textAlign(NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        return Context::render.text(pos.x, pos.y, text);
    }

    void FunctionTextLeft() {
        assert(stack.size() >= 3);
        auto* s(&stack.back() - 2);
        float end(FunctionTextLeftCommon(s[0].f, s[1].f, Context::strMng.get(s[2].l)));
        stack.resize(stack.size() - 2);
        stack.back().f = end;
    }

    void FunctionTextLeftCharPtr() {
        assert(stack.size() >= 3);
        auto* s(&stack.back() - 2);
        float end(FunctionTextLeftCommon(s[0].f, s[1].f, s[2].text));
        stack.resize(stack.size() - 2);
        stack.back().f = end;
    }

    void FunctionTextWidth() {
        assert(stack.size() >= 1);
        stack.back().f = Context::render.textWidth(Context::strMng.get(stack.back().l));
    }

    void FunctionTextWidthCPtr() {
        assert(stack.size() >= 1);
        stack.back().f = Context::render.textWidth(stack.back().text);
    }

    void FunctionTranslate() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        Context::render.translate(s[0].f, s[1].f);
        stack.resize(stack.size() - 2);
    }

    void FunctionScale() {
        assert(stack.size() >= 1);
        Context::render.scale(stack.back().f, stack.back().f);
        stack.pop_back();
    }

    void FunctionResetTransform() {
        Context::render.resetTransform();
    }

    void FunctionScissor() {
        assert(stack.size() >= 4);
        auto* s(&stack.back() - 3);
        Context::render.scissor(s[0].f, s[1].f, s[2].f, s[3].f);
        stack.resize(stack.size() - 4);
    }

    void FunctionResetScissor() {
        Context::render.resetScissor();
    }

    void FunctionQuery() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        if (!Context::app.executeQuery(s[0].strId, s[1].strId))
            DIAG(LOG("error executing query"));
        stack.clear(); // template execution could be messing with the stack: stack.resize(stack.size() - 2);
    }

    void FunctionTriggerTimers() {
        Context::app.triggerTimers();
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

    void FunctionAssignUint32() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        *reinterpret_cast<uint32_t*>(s[0].voidPtr) = s[1].u32;
        stack.resize(stack.size() - 2);
    }

    void FunctionAssignSizeRel() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        *reinterpret_cast<SizeRelative*>(s[0].voidPtr) = SizeRelative(s[1].f < 0 ? -s[1].f : s[1].f, s[1].f < 0);
        stack.resize(stack.size() - 2);
    }

    void FunctionAssignUint8() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        *reinterpret_cast<uint8_t*>(s[0].voidPtr) = uint8_t(s[1].f);
        stack.resize(stack.size() - 2);
    }

    void FunctionAssignInt16() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        *reinterpret_cast<int16_t*>(s[0].voidPtr) = int16_t(s[1].f);
        stack.resize(stack.size() - 2);
    }

    void FunctionAssignInt32() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        *reinterpret_cast<int32_t*>(s[0].voidPtr) = int32_t(s[1].f);
        stack.resize(stack.size() - 2);
    }

    void FunctionAssignText() {
        assert(stack.size() >= 2);
        auto* s(&stack.back() - 1);
        char*& text(*reinterpret_cast<char**>(s[0].voidPtr));
        free(text);
        text = strdup(s[1].text);
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
        FunctionProto func;
        Type* prototype;
        Type retType;
    } functionList[] = {
        { Identifier::InvalidId,       FunctionError,          VoidPrototype,         Type::LastType },
        { Identifier::beginPath,       FunctionBeginPath,      VoidPrototype,         Type::LastType },
        { Identifier::moveto,          FunctionMoveto,         Float2Prototype,       Type::LastType },
        { Identifier::lineto,          FunctionLineto,         Float2Prototype,       Type::LastType },
        { Identifier::bezierto,        FunctionBezierto,       Float6Prototype,       Type::LastType },
        { Identifier::closePath,       FunctionClosePath,      VoidPrototype,         Type::LastType },
        { Identifier::roundedRect,     FunctionRoundedRect,    Float5Prototype,       Type::LastType },
        { Identifier::fillColor,       FunctionFillColor,      ColorPrototype,        Type::LastType },
        { Identifier::fillVertGrad,    FunctionFillVertGrad,   FillVertGradPrototype, Type::LastType },
        { Identifier::fill,            FunctionFill,           VoidPrototype,         Type::LastType },
        { Identifier::strokeWidth,     FunctionStrokeWidth,    FloatPrototype,        Type::LastType },
        { Identifier::strokeColor,     FunctionStrokeColor,    ColorPrototype,        Type::LastType },
        { Identifier::stroke,          FunctionStroke,         VoidPrototype,         Type::LastType },
        { Identifier::font,            FunctionFont,           FontPrototype,         Type::LastType },
        { Identifier::text,            FunctionText,           TextFuncPrototype,     Type::Float },
        { Identifier::text,            FunctionTextCharPtr,    TextCharPtrPrototype,  Type::Float },
        { Identifier::textLeft,        FunctionTextLeft,       TextFuncPrototype,     Type::Float },
        { Identifier::textLeft,        FunctionTextLeftCharPtr,TextCharPtrPrototype,  Type::Float },
        { Identifier::textWidth,       FunctionTextWidth,      StrIdPrototype,        Type::Float },
        { Identifier::textWidth,       FunctionTextWidthCPtr,  TextPrototype,         Type::Float },
        { Identifier::translate,       FunctionTranslate,      Float2Prototype,       Type::LastType },
        { Identifier::scale,           FunctionScale,          FloatPrototype,        Type::LastType },
        { Identifier::resetTransform,  FunctionResetTransform, VoidPrototype,         Type::LastType },
        { Identifier::scissor,         FunctionScissor,        Float4Prototype,       Type::LastType },
        { Identifier::resetScissor,    FunctionResetScissor,   VoidPrototype,         Type::LastType },
        { Identifier::query,           FunctionQuery,          QueryPrototype,        Type::LastType },
        { Identifier::triggerTimers,   FunctionTriggerTimers,  VoidPrototype,         Type::LastType },
        { Identifier::log,             FunctionLog,            StrIdPrototype,        Type::LastType },
        { Identifier::add,             FunctionAdd,            Float2Prototype,       Type::Float },
        { Identifier::sub,             FunctionSub,            Float2Prototype,       Type::Float },
        { Identifier::mul,             FunctionMul,            Float2Prototype,       Type::Float },
        { Identifier::div,             FunctionDiv,            Float2Prototype,       Type::Float },
        { Identifier::mod,             FunctionMod,            ModPrototype,          Type::Color },
        { Identifier::assign,          FunctionAssignUint32,   AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignSizeRel,  AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignUint8,    AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignInt16,    AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignInt32,    AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignText,     AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignBit<0>,   AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignBit<1>,   AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignBit<2>,   AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignBit<3>,   AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignBit<4>,   AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignBit<5>,   AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignBit<6>,   AssignPrototype,       Type::LastType },
        { Identifier::assign,          FunctionAssignBit<7>,   AssignPrototype,       Type::LastType },
    };

    DIAG(const char* toString(Function func) {
            return Context::strMng.get(functionList[int(func)].id);
        });

    inline Function funcById(Identifier id) {
        // TODO: do binary search
        for (const auto& f: functionList)
            if (f.id == id) return Function(&f - functionList);
        return Function::Error;
    }

    Widget* ancestor(Widget* w, int level) {
        while (level--) {
            w = w->parent;
            DIAG(if (!w) LOG("internal: run out of parents while resoling parent property"));
        }
        return w;
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
        //DIAG(LOG("execute on %p: %s", widget, DryRun ? "dry run" : "for real"); dump(iAction));
        stack.clear();
        execWidget = widget;
        if (DryRun) locations.clear();
        while (true) {
            const auto& action(actions[iAction]);
            switch (action.inst()) {
            case Instruction::Return:
                //DIAG(if (!DryRun) LOG(""));
                return true;
            case Instruction::Nop:
                break;
            case Instruction::PushConstant:
                stack.push_back(StackFrame(actions[iAction + 1].l));
                if (DryRun) locations.push_back(iAction);
                if (action.param) {
                    stack.push_back(StackFrame(actions[iAction + 2].l));
                    if (DryRun) locations.push_back(iAction);
                    ++iAction;
                }
                ++iAction;
                break;
            case Instruction::PushProperty:
                stack.push_back(StackFrame(getPropertyData(widget, action)));
                if (DryRun) locations.push_back(iAction);
                break;
            case Instruction::PushForeignProperty:
                stack.push_back(StackFrame(getPropertyData(actions[iAction + 1].widget, action)));
                if (DryRun) locations.push_back(iAction);
                ++iAction;
                break;
            case Instruction::PushDoubleProperty: {
                auto* resolved(resolveDoubleDispatch(actions[iAction + 1].l, widget));
                DIAG(if (!resolved) { LOG("double dispatch failed"); return false; });
                stack.push_back(StackFrame(getPropertyData(resolved, action)));
                if (DryRun) locations.push_back(iAction);
                iAction += 2;
                break;
            }
            case Instruction::PushParentProperty: {
                auto* parent(ancestor(widget, actions[iAction + 1].l));
                stack.push_back(StackFrame(getPropertyData(parent, action)));
                if (DryRun) locations.push_back(iAction);
                break;
            }
            case Instruction::PushDoubleParentProperty: {
                auto* parent(ancestor(widget, actions[iAction + 2].l));
                auto* resolved(resolveDoubleDispatch(actions[iAction + 1].l, parent));
                DIAG(if (!resolved) { LOG("double parent dispatch failed"); return false; });
                stack.push_back(StackFrame(getPropertyData(resolved, action)));
                if (DryRun) locations.push_back(iAction);
                iAction += 2;
                break;
            }
            case Instruction::PushPropertyPtr:
                stack.push_back(StackFrame(long(widget) + action.param));
                if (DryRun) locations.push_back(iAction);
                break;
            case Instruction::PushForeignPropertyPtr:
                stack.push_back(StackFrame(actions[iAction + 1].l + action.param));
                if (DryRun) locations.push_back(iAction);
                ++iAction;
                break;
            case Instruction::PushDoublePropertyPtr: {
                auto* resolved(resolveDoubleDispatch(actions[iAction + 1].l, widget));
                DIAG(if (!resolved) { LOG("double dispatch ptr failed"); return false; });
                stack.push_back(StackFrame(long(resolved) + action.param));
                if (DryRun) locations.push_back(iAction);
                iAction += 2;
                break;
            }
            case Instruction::PushParentPropertyPtr: {
                auto* parent(ancestor(widget, actions[iAction + 1].l));
                stack.push_back(StackFrame(long(parent) + action.param));
                if (DryRun) locations.push_back(iAction);
                break;
            }
            case Instruction::PushDoubleParentPropertyPtr: {
                auto* parent(ancestor(widget, actions[iAction + 2].l));
                auto* resolved(resolveDoubleDispatch(actions[iAction + 1].l, parent));
                DIAG(if (!resolved) { LOG("double parent dispatch ptr failed"); return false; });
                stack.push_back(StackFrame(long(resolved) + action.param));
                if (DryRun) locations.push_back(iAction);
                iAction += 2;
                break;
            }
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

    bool Actions::evalProperty(MLParser& parser, int iEntry, int fEntry, StringId propId, Widget* widget, bool onlyIfTemplated, bool define) {
        // creating actions: prop = expression
        int iAction(actions.size());
        actions.push_back(Command(Instruction::PushConstant, Type::Id));
        actions.push_back(Command(propId));
        long dispatchParam;
        DispatchType dispatchType;
        const auto* prop(resolveProperty(&actions[iAction], widget, dispatchType, dispatchParam));
        if (!prop) {
            DIAG(LOG("no such property in widget: %s", Context::strMng.get(propId)));
            return false;
        }
        if (prop->type == Type::ActionTable) actions.resize(iAction); // action
        defining = define;
        templateFound = false;
        if (!add(parser, iEntry, fEntry)) {
            DIAG(LOG("evaluating property: hasTemplates=%d defining=%d", templateFound, defining));
            return templateFound && defining; // templatized attribute of template while defining
        }
        if (!onlyIfTemplated || templateFound) {
            if (prop->type == Type::ActionTable) {
                // action
                Context::app.addAction(Identifier(propId.getId()), iAction, widget);
            } else {
                actions.pop_back(); // remove return
                actions.push_back(Command(Instruction::FunctionCall, Type::LastType, Function::AssignUint32));
                actions.push_back(Command(Instruction::Return));
                // prepare expression
                if (!execute<true>(iAction, widget)) {
                    DIAG(LOG("preparing evaluation of property"));
                    return false;
                }
                if (prop->type == Type::Text) { // special case text
                    // as text cannot be operated with, just set the value and exit
                    // this avoids a strdup
                    auto* com(&actions[iAction]);
                    *(const char**)((char*)widget + com[0].param) = com[3].text;
                } else {
                    // execution
                    if (!execute(iAction, widget)) {
                        DIAG(LOG("executing evaluation of property"));
                        return false;
                    }
                }
                // remove operation
                actions.resize(iAction);
            }
        } else {
            // remove operation
            actions.resize(iAction);
        }
        return true;
    }

    bool Actions::addRecur(MLParser& parser, int iEntry, int fEntry) {
        bool attr;
        while (iEntry < fEntry) {
            // function, expression or assignment expected
            bool recur(true);
            const auto* entry(&parser[iEntry]);
            switch (entry->type()) {
            case MLParser::EntryType::Id:
                // cannot resolve the id (widget/property) at this point
                attr = (parser[iEntry + 1].type() == MLParser::EntryType::Attribute);
                actions.push_back(Command(Instruction::PushConstant, Type::Id, attr));
                actions.push_back(Command(Context::strMng.add(entry->pos, parser.size(iEntry))));
                if (attr) {
                    entry = &parser[++iEntry];
                    actions.push_back(Command(Context::strMng.add(entry->pos, parser.size(iEntry))));
                }
                break;
            case MLParser::EntryType::Number:
                // just push it
                actions.push_back(Command(Instruction::PushConstant, Type::Float));
                actions.push_back(Command(float(atof(entry->pos))));
                break;
            case MLParser::EntryType::Operator:
            case MLParser::EntryType::Function:
                recur = false;
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
                actions.push_back(Command(Instruction::PushConstant, Type::StrView, 1));
                actions.push_back(Command(entry->pos + 1));
                actions.push_back(Command(long(parser.size(iEntry) - 2)));
                break;
            case MLParser::EntryType::Wildcar: {
                int iTpl, fTpl;
                templateFound = true;
                if (defining) return false;
                if (!Context::app.startTemplate(iTpl, fTpl) ||
                    !addRecur(Context::app.getTemplateParser(), iTpl, fTpl) ||
                    !Context::app.endTemplate()) return false;
                break;
            }
            default:
                DIAG(LOG("invalid ML entry type: %s", MLParser::toString(entry->type())));
                return false;
            }
            if (recur && !addRecur(parser, iEntry + 1, entry->next)) return false;
            iEntry = entry->next;
        }
        return true;
    }

    const Property* Actions::resolveProperty(Command* command, Widget* widget, DispatchType& type, long& param) {
        assert(command->inst() == Instruction::PushConstant && command->type() == Type::Id);
        StringId propId;
        if (command->param) {
            // x.y => widget.propery
            auto widgetId(command[1].strId);
            assert(widgetId.valid());
            if (widgetId == Identifier::parent) {
                // explicit parent: parent.property
                DIAG(if (!widget->parent) { LOG("no parent for widget when explicitly stated"); return nullptr; });
                propId = command[2].strId;
                DIAG(if (!propId.valid()) LOG("invalid property id"));
                // property
                widget = widget->parent;
                const Property* prop(widget->getProp(Identifier(propId.getId())));
                type = DispatchParent;
                param = 1;
                return prop;
            } else if (!Context::app.getWidgets().count(widgetId)) {
                // maybe it's a property of the widget (or ancestors) to refer a widget
                const Property* prop(widget->getProp(Identifier(widgetId.getId())));
                Widget* parent(widget);
                param = 0;
                while (!prop && parent->parent) { // try to find the property in hierarchy
                    parent = parent->parent;
                    param++;
                    prop = parent->getProp(Identifier(widgetId.getId()));
                }
                if (!prop || prop->type != Type::Id || // variable has to be set to a valid widget since the beginning
                    !Context::app.getWidgets().count(widgetId = reinterpret_cast<StringId*>(parent)[prop->pos])) {
                    DIAG(LOG("unknown widget: %s.%s resolving property (%s = %s [%d])",
                             Context::strMng.get(command[1].strId), Context::strMng.get(command[2].strId),
                             Context::strMng.get(command[1].strId),
                             prop ? Context::strMng.get(reinterpret_cast<StringId*>(parent)[prop->pos]) : "error",
                             prop ? reinterpret_cast<int*>(parent)[prop->pos] : -1));
                    type = DispatchUnknown;
                    return nullptr;
                }
                widget = Context::app.getWidgets()[widgetId];
                if (param && parent) {
                    // double dispatch parent: (ancestor:variable->widget).property
                    type = DispatchDoubleParent;
                    param = prop->pos | param << 20;
                } else {
                    // double dispatch: (variable->widget).property
                    type = DispatchDouble;
                    param = prop->pos;
                }
            } else {
                // foreign: widget.property
                widget = Context::app.getWidgets()[widgetId];
                type = DispatchForeign;
                param = long(widget);
            }
            propId = command[2].strId;
            DIAG(if (!propId.valid()) LOG("invalid property id"));
            // property
            return widget->getProp(Identifier(propId.getId()));
        } else {
            propId = command[1].strId;
            DIAG(if (!propId.valid()) LOG("invalid property id"));
            // property
            const Property* prop(nullptr);
            Widget* parent(widget);
            param = 0;
            while (parent) { // try to find the property in hierarchy
                if ((prop = parent->getProp(Identifier(propId.getId())))) break;
                parent = parent->parent;
                param++;
            }
            if (param && parent) {
                // parent property
                type = DispatchParent;
            } else {
                // normal: property
                type = DispatchNormal;
            }
            return prop;
        }
    }

    void Actions::resolvePropertyRecode(const Property* prop, DispatchType type, long param, Command* command, bool ptr) {
        auto pos(!ptr && prop->type == Type::Bit ? (prop->pos << 3 | prop->bit) : ptr ? prop->pos * prop->size : prop->pos);
        int instBase = ptr ? int(Instruction::PushPropertyPtr) : int(Instruction::PushProperty);
        int commandSize(1 + 1 + command->param);
        int commandNow(1);
        command[0] = Command(Instruction(instBase + type), prop->type, pos);
        switch (type) {
        case DispatchNormal:
            break;
        case DispatchForeign:
            command[1] = Command(param);           // foreign widget
            commandNow++;
            break;
        case DispatchDouble:
            command[1] = Command(param);           // variable id position in widget
            commandNow++;
            break;
        case DispatchParent:
            command[1] = Command(param);           // ancestor level
            commandNow++;
            break;
        case DispatchDoubleParent:
            command[1] = Command(param & 0xfffff); // variable id position in widget
            command[2] = Command(param >> 20);     // ancestor level
            commandNow += 2;
            break;
        case DispatchUnknown:
        default:
            DIAG(LOG("bad dispatching"));
            break;
        }
        assert(commandNow <= commandSize);
        while (commandNow < commandSize)
            command[commandNow++] = Command(Instruction::Nop);
    }

    Widget* Actions::resolveDoubleDispatch(int pos, Widget* widget) {
        auto widgetId(reinterpret_cast<StringId*>(widget)[pos]);
        assert(widgetId.valid());
        DIAG(if (!Context::app.getWidgets().count(widgetId)) {
                LOG("failed double dispatch, cannot find widget: %s", Context::strMng.get(widgetId));
                return nullptr;
            });
        return Context::app.getWidgets()[widgetId];
    }

    long Actions::getPropertyData(const void* data, Command command) {
        float f;
        switch (command.type()) {
        case Type::Bit:          f = (reinterpret_cast<const uint8_t*>(data)[command.param >> 3] >> (command.param & 7)) & 1; break;
        case Type::Uint8:        f = reinterpret_cast<const uint8_t*>(data)[command.param]; break;
        case Type::Int16:        f = reinterpret_cast<const int16_t*>(data)[command.param]; break;
        case Type::Int32:        f = reinterpret_cast<const int32_t*>(data)[command.param]; break;
        case Type::SizeRelative: f = reinterpret_cast<const int16_t*>(data)[command.param] & 0x3fff; break;
        case Type::Id:
        case Type::StrId:
        case Type::Float:
        case Type::Color:
        case Type::FontIdx:
                             return reinterpret_cast<const uint32_t*>(data)[command.param];
        case Type::Text:
        case Type::VoidPtr:
                             return reinterpret_cast<const long*>(data)[command.param];
        default:
            DIAG(LOG("unmanaged property type for reading: %s", toString(command.type())));
            abort();
        }
        // promotion to float
        return *reinterpret_cast<const uint32_t*>(&f);
    }

    bool Actions::checkFunctionParams(int iFunction, int iAction, Widget* widget) {
        DIAG(int stackSizeOrig(stack.size()));
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
            long dispatchParam;
            DispatchType dispatchType;
            const Property* prop(command->inst() == Instruction::PushConstant && type == Type::Id ?
                                 resolveProperty(command, widget, dispatchType, dispatchParam) : nullptr);
            if (type != *proto) {
                // try to do the execution conversion
                if (*proto == Type::VoidPtr) {
                    // assign or inline expressions
                    if (!prop) {
                        if (command->inst() != Instruction::PushConstant) return true; // already recoded: second pass
                        DIAG(if (!prop) LOG("property id '%s' not available on widget at %d",
                                            Context::strMng.get(command[1].strId), locations[iStack]));
                        return false;
                    }
                    // upgrade value types
                    auto* value(&actions[locations[iStack + 1]]);
                    auto valueType(value[0].type());
                    if ((prop->type == Type::Bit          && valueType == Type::Float) ||
                        (prop->type == Type::Uint8        && valueType == Type::Float) ||
                        (prop->type == Type::Int16        && valueType == Type::Float) ||
                        (prop->type == Type::Int32        && valueType == Type::Float) ||
                        (prop->type == Type::SizeRelative && valueType == Type::Float) ||
                        (prop->type == Type::Float        && valueType == Type::Bit)   ||
                        (prop->type == Type::Float        && valueType == Type::Int16) ||
                        (prop->type == Type::Float        && valueType == Type::SizeRelative) ||
                        (prop->type == Type::StrId        && valueType == Type::Id))   valueType = prop->type;
                    else if (prop->type == Type::Text && valueType == Type::StrView) {
                        // promote string view to text
                        value[0].sub = int(Type::Text);
                        value[0].param = 0;
                        value[1].text = strndup(value[1].text, value[2].l);
                        value[2] = Command(Instruction::Nop);
                        valueType = prop->type;
                    }
                    if (prop->type == valueType) {
                        resolvePropertyRecode(prop, dispatchType, dispatchParam, command, true);
                        Function assignFunc;
                        switch (prop->type) {
                        case Type::Bit:          assignFunc = Function(int(Function::AssignBit0) + prop->bit); break;
                        case Type::SizeRelative: assignFunc = Function::AssignSizeRel; break;
                        case Type::Uint8:        assignFunc = Function::AssignUint8; break;
                        case Type::Int16:        assignFunc = Function::AssignInt16; break;
                        case Type::Int32:        assignFunc = Function::AssignInt32; break;
                        case Type::Text:         assignFunc = Function::AssignText; break;
                        case Type::Float:
                        case Type::Color:
                        case Type::Id:
                        case Type::StrId:
                                                 assignFunc = Function::AssignUint32; break;
                        default: DIAG(LOG("unknown assignment")); return false;
                        }
                        actions[iAction] = Command(Instruction::FunctionCall, prop->type, assignFunc);
                    } else {
                        DIAG(LOG("cannot cast '%s' from %s to %s in %d (assignment)",
                                 Context::strMng.get(command[1].strId), toString(valueType), toString(prop->type), iAction));
                        return false;
                    }
                } else if (prop) {
                    // cast push constant id to push property
                    bool cast(true);
                    if (prop->type != *proto && *proto != Type::Unknown) {
                        if (*proto == Type::Float &&
                            (prop->type == Type::Bit   || prop->type == Type::Uint8 ||
                             prop->type == Type::Int16 || prop->type == Type::Int32 || prop->type == Type::SizeRelative)) {
                            // these types are promoted to float, so allow cast
                        } else if (iFunction == int(Function::Text) && prop->type == Type::Text) {
                            // change function from text to textCharPtr
                            actions[iAction].param = int(Function::TextCharPtr);
                        } else if (iFunction == int(Function::TextLeft) && prop->type == Type::Text) {
                            // change function from text to textCharPtr
                            actions[iAction].param = int(Function::TextLeftCharPtr);
                        } else if (iFunction == int(Function::TextWidth) && prop->type == Type::Text) {
                            // change function from text to textCharPtr
                            actions[iAction].param = int(Function::TextWidthCharPtr);
                        } else
                            // no polymorphism found
                            cast = false;
                    }

                    if (cast) {
                        // conversion of type ok
                        resolvePropertyRecode(prop, dispatchType, dispatchParam, command, false);
                    } else {
                        DIAG(LOG("cannot cast '%s' from %s to %s in %d",
                                 Context::strMng.get(command[1].strId), toString(prop->type), toString(*proto), iAction));
                        return false;
                    }
                } else if (*proto == Type::Unknown) {
                    // assign right hand side
                } else if (command->inst() == Instruction::PushConstant && type == Type::StrView) {
                    // cast to strId
                    command[0].sub = int(Type::StrId);
                    command[0].param = 0;
                    command[1].l = int(Context::strMng.add(command[1].text, command[2].l).getId());
                    command[2] = Command(Instruction::Nop);
                    if (*proto == Type::FontIdx) {
                        // cast font name to font index
                        command[0].sub = int(Type::FontIdx);
                        command[1].l = Context::app.getFont(command[1].strId);
                    }
                } else {
                    // cannot cast
                    DIAG(
                        LOG("expected parameter %d of type %s, got %s in %s at position %d",
                            stackSizeOrig - iStack, toString(*proto), toString(type), Context::strMng.get(func.id), locations[iStack]);
                        dumpStack());
                    return false;
                }
            } else {
                // type matches proto, but if pushing a constant id and this is a property of type id, then resolve
                if (prop && prop->type == Type::Id)
                    resolvePropertyRecode(prop, dispatchType, dispatchParam, command, false);
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
            stack.push_back(StackFrame(0.0f));
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
                        actions[i].param && actions[i].type() != Type::StrView ?
                        ::toString(actions[i].type(), &actions[i + 2], buffer2, sizeof(buffer2)) : "");
                    i += 1 + actions[i].param;
                    break;
                case Instruction::PushProperty:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d)",
                        i, "Push prop", ::toString(actions[i].type()), actions[i].param);
                    break;
                case Instruction::PushForeignProperty:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d), widget(%p)",
                        i, "Push foreign prop", ::toString(actions[i].type()), actions[i].param, actions[i+1].widget);
                    i++;
                    break;
                case Instruction::PushDoubleProperty:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d), variable pos(%ld)",
                        i, "Push double prop", ::toString(actions[i].type()), actions[i].param, actions[i+1].l);
                    i++;
                    break;
                case Instruction::PushParentProperty:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d), parentAncestor(%ld)",
                        i, "Push parent prop", ::toString(actions[i].type()), actions[i].param, actions[i+1].l);
                    i++;
                    break;
                case Instruction::PushDoubleParentProperty:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d), variable pos(%ld) parentAncestor(%ld)",
                        i, "Push 2 parent prop", ::toString(actions[i].type()), actions[i].param, actions[i+1].l, actions[i+2].l);
                    i += 2;
                    break;
                case Instruction::PushPropertyPtr:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d)",
                        i, "Push prop ptr", ::toString(actions[i].type()), actions[i].param);
                    break;
                case Instruction::PushForeignPropertyPtr:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d), widget(%p)",
                        i, "Push foreign prop ptr", ::toString(actions[i].type()), actions[i].param, actions[i+1].widget);
                    i++;
                    break;
                case Instruction::PushDoublePropertyPtr:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d), variable pos(%ld)",
                        i, "Push double prop ptr", ::toString(actions[i].type()), actions[i].param, actions[i+1].l);
                    i++;
                    break;
                case Instruction::PushParentPropertyPtr:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d), parentAncestor(%ld)",
                        i, "Push parent prop ptr", ::toString(actions[i].type()), actions[i].param, actions[i+1].l);
                    i++;
                    break;
                case Instruction::PushDoubleParentPropertyPtr:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d), variable pos(%ld) parentAncestor(%ld)",
                        i, "Push 2 parent ptr", ::toString(actions[i].type()), actions[i].param, actions[i+1].l, actions[i+2].l);
                    i += 2;
                    break;
                case Instruction::FunctionCall:
                    LOG("%6d " GREEN "%-20s " RESET ": func(" CYAN "%s" RESET ") -> %s   type(%s)",
                        i, "Function call", ::toString(Function(actions[i].param)), toString(functionList[actions[i].param].retType), toString(com.type()));
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
            for (int i = 0; i < int(stack.size()); i++)
                LOG("%4d " GREEN "float(%f), uint32_t(%ld), ptr(%p)" RESET,
                    i, stack[i].f, stack[i].l, stack[i].voidPtr);
        });

    // explicit template instantiations
    template bool Actions::execute<true>(int iAction, Widget* widget);
    template bool Actions::execute<false>(int iAction, Widget* widget);

}
