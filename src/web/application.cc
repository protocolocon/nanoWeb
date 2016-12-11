/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "input.h"
#include "widget.h"
#include "context.h"
#include "properties.h"
#include "widget_timer.h"
#include "reserved_words.h"
#include "widget_layout.h"
#include "widget_template.h"
#include "widget_application.h"
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <algorithm>

using namespace std;
using namespace webui;

namespace {

    const Type logParams[] =             { Type::StrId, Type::LastType };
    const Type toggleVisibleParams[] =   { Type::StrId, Type::LastType };
    const Type beginPathParams[] =       { Type::LastType };
    const Type movetoParams[] =          { Type::Coord, Type::Coord, Type::LastType };
    const Type linetoParams[] =          { Type::Coord, Type::Coord, Type::LastType };
    const Type beziertoParams[] =        { Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::LastType };
    const Type closePathParams[] =       { Type::LastType };
    const Type roundedRectParams[] =     { Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::LastType };
    const Type fillColorParams[] =       { Type::ColorModif, Type::LastType };
    const Type fillVertGradParams[] =    { Type::Coord, Type::Coord, Type::ColorModif, Type::ColorModif, Type::LastType };
    const Type fillParams[] =            { Type::LastType };
    const Type strokeWidthParams[] =     { Type::Float, Type::LastType };
    const Type strokeColorParams[] =     { Type::ColorModif, Type::LastType };
    const Type strokeParams[] =          { Type::LastType };
    const Type fontParams[] =            { Type::FontIdx, Type::Float, Type::LastType };
    const Type textParams[] =            { Type::Coord, Type::Coord, Type::TextPropOrStrId, Type::LastType };
    const Type translateCenterParams[] = { Type::LastType };
    const Type scale100Params[] =        { Type::Coord, Type::LastType };
    const Type resetTransformParams[] =  { Type::LastType };
    const Type setParams[] =             { Type::StrId, Type::Id, Type::Str, Type::LastType };
    const Type queryParams[] =           { Type::StrId, Type::StrId, Type::LastType };

}

namespace webui {

    Application::Application(Context& ctx): ctx(ctx), iTplProp(0), fTplProp(0), layoutStable(false),
                                            actionTables(1), actionCommands(1, int(Identifier::CLast)), root(nullptr),
                                            fontValid(false) {
        addReservedWords(strMng);
    }

    Application::~Application() {
        clear();
        for (auto& font: fonts)
            free(font.second);
    }

    void Application::initialize() {
        new RequestXHR(*this, RequestXHR::TypeApplication, StringId(), "application.ml");
    }

    void Application::refresh() {
        if (root && ((Input::refresh(ctx.getRender().getWin()) | refreshTimers()) || !layoutStable)) {
            layoutStable = root->layout(ctx, V2s(0, 0), V2s(ctx.getRender().getWidth(), ctx.getRender().getHeight()));
            ctx.forceRender();
        }
    }

    bool Application::refreshTimers() {
        bool dev(false);
        for (auto child: root->getChildren())
            if (child->type() == Identifier::Timer)
                dev |= reinterpret_cast<WidgetTimer*>(child)->refreshTimer(ctx);
        return dev;
    }

    bool Application::update() {
        assert(root);
        return root->update(*this);
    }

    void Application::clear() {
        root = nullptr;
        for (auto& widget: widgets)
            delete widget.second;
        widgets.clear();
    }

    void Application::resize(int width, int height) {
        if (root) {
            ctx.resetRatio();
            root->layout(ctx, V2s(0, 0), V2s(ctx.getRender().getWidth(), ctx.getRender().getHeight()));
        }
    }

    void Application::render() {
        if (root) {
            auto& render(ctx.getRender());
            render.beginFrame();
            root->render(ctx, 0x100);
            ctx.getRender().endFrame();
        }
    }

    void Application::onLoad(RequestXHR* xhr) {
        switch (xhr->getType()) {
        case RequestXHR::TypeApplication:
            assert(!root);
            if (!tree.parse(xhr->getData(), xhr->getNData()) || !(root = initializeConstruct())) {
                DIAG(
                    xhr->makeCString();
                    LOG("cannot parse: %s", xhr->getData()));
                clear();
            } else
                resize(ctx.getRender().getWidth(), ctx.getRender().getHeight());

            tree.clear();
            ctx.forceRender();
            DIAG(
                //tree.dumpTree();
                dump());
            break;
        case RequestXHR::TypeTemplate: {
            // get template widget
            auto it(widgets.find(xhr->getId()));
            DIAG(
                if (it == widgets.end())
                    LOG("internal: cannot find template %s", strMng.get(xhr->getId())));
            auto* tplWidget(reinterpret_cast<WidgetTemplate*>(it->second));
            tree.swap(tplWidget->getParser());
            bool define;
            if (!tpl.parse(xhr->getData(), xhr->getNData(), true/*value*/) ||
                !initializeConstruct(tplWidget, 0, tree.size(), 0, tpl.size(), define, true)) {
                LOG("error: update template");
            }
            tplWidget->setVisible(true);
            DIAG(
                tree.dump();
                tree.dumpTree();
                LOG("----------");
                tpl.dump();
                tpl.dumpTree();
                LOG("----------");
                dump());
            tree.swap(tplWidget->getParser());
            layoutStable = false;
            ctx.forceRender();
            break;
        }
        case RequestXHR::TypeFont: {
            // check that font id matches in font list
            bool ok(false);
            StringId id(xhr->getId());
            for (auto& font: fonts)
                if (font.first == id) {
                    assert(!font.second);
                    // need to copy the font buffer
                    font.second = (char*)malloc(xhr->getNData());
                    memcpy(font.second, xhr->getData(), xhr->getNData());
                    int vgId(ctx.getRender().loadFont(strMng.get(id), font.second, xhr->getNData()));
                    if (vgId == &font - fonts.data())
                        ok = true;
                    break;
                }
            if (!ok)
                LOG("internal error: font indices: %s", strMng.get(id));
            ctx.forceRender();
            break;
        }
        default:
            DIAG(
                xhr->makeCString();
                LOG("internal error: unknown query: %s %s", strMng.get(xhr->getId()), xhr->getData()));
            break;
        }
        delete xhr;
    }

    void Application::onError(RequestXHR* xhr) {
        LOG("lost query: %s", strMng.get(xhr->getId()));
        delete xhr;
    }

    Widget* Application::initializeConstruct() {
        if (tree.size() > 1 && tree[0].next == 1 && tree[1].next == 0 && *tree[1].pos == '{') {
            bool define;
            auto* widget(createWidget(tree[0].asId(tree, strMng), nullptr));
            if (widget && initializeConstruct(widget, 2, tree.size(), -1, -1, define, true) && registerWidget(widget))
                return widget;
            delete widget;
        }
        return nullptr;
    }

    bool Application::initializeConstruct(Widget* widget, int iEntry, int fEntry, int iTpl, int fTpl, bool& define, bool recurse) {
        define = false;
        while (iEntry < fEntry) {
            // key : value
            const auto& entryKey(tree[iEntry]);
            assert(iEntry + 1 < fEntry && entryKey.next == iEntry + 1);
            auto& entryVal(tree[iEntry + 1]);
            // key and next after value
            auto key(entryKey.asId(tree, strMng));
            int next(entryVal.next ? entryVal.next : fEntry);
            if (key == Identifier::InvalidId) {
                DIAG(
                    auto ss(entryKey.asStrSize(tree, true));
                    LOG("error: invalid identifier {%.*s}", ss.second, ss.first);
                    tree.error(entryKey.pos, "=>", entryKey.line));
            } else if (isWidget(key)) {
                if (recurse) {
                    int iIter(-1), fIter(0);
                    if (iTpl >= 0) {
                        if (iTpl < fTpl) {
                            if (*tpl[iTpl].pos == '[') { // list of widgets
                                iIter = iTpl + 1;
                                fIter = tpl[iTpl].next ? tpl[iTpl].next : fTpl;
                                iTpl = fIter; // skip the widget list
                            } else {
                                DIAG(
                                    LOG("expected list of widgets");
                                    tpl.error(tpl[iTpl].pos, "=>", tpl[iTpl].line));
                            }
                        } else {
                            DIAG(LOG("run out of template values while creating child %d %d", iTpl, fTpl));
                            return false;
                        }
                    }
                    while (iIter < fIter) {
                        // template widget
                        int iWidget(-1), fWidget(-1);
                        if (iIter >= 0) {
                            if (*tpl[iIter].pos == '[') { // widget
                                iWidget = iIter + 1;
                                fWidget = tpl[iIter].next ? tpl[iIter].next : fIter;
                            } else {
                                DIAG(
                                    LOG("expected widget []");
                                    tpl.error(tpl[iIter].pos, "=>", tpl[iIter].line));
                            }
                        }
                        // child widget
                        Widget* widgetChild(createWidget(key, widget/*parent*/));
                        assert(widgetChild);
                        bool defineChild(false);
                        bool isTemplate(key == Identifier::Template);
                        if (!initializeConstruct(widgetChild, iEntry + 2, next, iWidget, fWidget, defineChild, !isTemplate) ||
                            !registerWidget(widgetChild)) {
                            delete widgetChild;
                            return false;
                        }
                        if (!defineChild) widget->addChild(widgetChild);
                        if (isTemplate) {
                            auto* tpl(reinterpret_cast<WidgetTemplate*>(widgetChild));
                            tree.copyTo(tpl->getParser(), iEntry + 2, next);
                        }
                        // prepare next template widget
                        if (iIter >= 0)
                            iIter = fWidget; // next widget
                        else
                            break; // normal construction
                    }
                }
            } else {
                // check for @
                bool templateReplaced(replaceProperty(iEntry + 1, iTpl, fTpl));
                if (key == Identifier::define) {
                    key = Identifier::id;
                    define = true;
                }
                if (!setProp(key, widget, iEntry + 1, next)) {
                    DIAG(
                        auto ss(entryVal.asStrSize(tree, true));
                        LOG("warning: unknown attribute %s with value %.*s", strMng.get(StringId(int(key))), ss.second, ss.first);
                        tree.error(entryKey.pos, "=>", entryKey.line));
                }
                iTpl = replaceBackProperty(templateReplaced, iEntry + 1, iTpl);
            }
            iEntry = next;
        }
        DIAG(
            if (iTpl < fTpl)
                LOG("trailing template values after widget"));
        return true;
    }

    bool Application::replaceProperty(int iEntry, int iTpl, int fTpl) {
        iTplProp = iTpl;
        fTplProp = fTpl;
        auto& entry(tree[iEntry]);
        if (entry.pos[0] == '@') {
            if (iTpl < fTpl) {
                swap(entry.pos, tpl[iTpl].pos);
                tree.swapEnd(tpl);
                return true;
            } else {
                DIAG(
                    if (iTpl != -1 || fTpl != -1) {
                        LOG("out of template values: %d %d", iTpl, fTpl);
                        tree.error(entry.pos, "=>", entry.line);
                    });
            }
        }
        return false;
    }

    int Application::replaceBackProperty(bool templateReplaced, int iEntry, int iTpl) {
        int iTplNew;
        if (templateReplaced) {
            swap(tree[iEntry].pos, tpl[iTpl].pos);
            tree.swapEnd(tpl);
            iTplNew = iTpl + 1;
        } else
            iTplNew = iTplProp;
        iTplProp = fTplProp = -1;
        return iTplNew;
    }

    bool Application::isWidget(Identifier id) const {
        return id < Identifier::WLast || widgets.find(int(id)) != widgets.end();
    }

    Widget* Application::createWidget(Identifier id, Widget* parent) {
        switch (id) {
        case Identifier::Application: return new WidgetApplication(parent);
        case Identifier::Widget:      return new Widget(parent);
        case Identifier::LayoutHor:   return new WidgetLayout(0, parent);
        case Identifier::LayoutVer:   return new WidgetLayout(1, parent);
        case Identifier::Template:    return new WidgetTemplate(parent);
        case Identifier::Timer:       return new WidgetTimer(parent);
        default:                      break;
        }
        // copy from other widget by id
        auto it(widgets.find(int(id)));
        if (it != widgets.end()) {
            auto* widget(createWidget(it->second->type(), parent));
            widget->copyFrom(it->second);
            return widget;
        }
        return nullptr;
    }

    bool Application::registerWidget(Widget* widget) {
        if (!widget->getId().valid()) {
            // set an internal id
            static int id(0);
            char buffer[16];
            int nBuffer(snprintf(buffer, sizeof(buffer), "@%x", id++));
            widget->setId(strMng.add(buffer, nBuffer));
        }
        const auto& id(widget->getId());
        if (widgets.count(id)) {
            DIAG(LOG("repeated widget id: %s", strMng.get(id)));
            return false;
        }
        widgets[id] = widget;
        return true;
    }

    bool Application::setProp(Identifier id, Widget* widget, int iEntry, int fEntry) {
        const auto* prop(widget->getProp(id));
        if (!prop) {
            DIAG(LOG("invalid property '%s' for widget of type: %s", strMng.get(id), strMng.get(widget->type())));
            return false;
        }
        if (!setProp(*prop, id, widget, iEntry, fEntry, widget)) {
            DIAG(
                auto ss(entryAsStrSize(iEntry, true));
                LOG("%s was expecting the type %s, received %.*s (%d parameters) and failed",
                    strMng.get(id), toString(prop->type), ss.second, ss.first, fEntry - iEntry));
            return false;
        }
        return true;
    }

    bool Application::setProp(const Property& prop, Identifier id, void* data, int iEntry, int fEntry, Widget* widget) {
        pair<const char*, int> ss;
        switch (prop.type) {
        case Type::Bit:
            DIAG(if (fEntry > iEntry + 1) return false);
            if (*tree[iEntry].pos == '0')
                reinterpret_cast<uint8_t*>(data)[prop.pos] &= ~(1 << prop.bit);
            else
                reinterpret_cast<uint8_t*>(data)[prop.pos] |= 1 << prop.bit;
            return true;
        case Type::Uint8:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<uint8_t*>(data)[prop.pos] = atoi(tree[iEntry].pos);
            return true;
        case Type::Int32:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<int*>(data)[prop.pos] = atoi(tree[iEntry].pos);
            return true;
        case Type::Id:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<Identifier*>(data)[prop.pos] = entryId(iEntry);
            return true;
        case Type::Str:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<StringId*>(data)[prop.pos] = entryAsStrId(iEntry, true);
            return true;
        case Type::StrId:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<StringId*>(data)[prop.pos] = entryAsStrId(iEntry, false);
            return true;
        case Type::Float:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<float*>(data)[prop.pos] = strtof(tree[iEntry].pos, nullptr);
            return true;
        case Type::Color:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<RGBA*>(data)[prop.pos] = entry(iEntry).pos;
            return true;
        case Type::ColorModif:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<RGBAref*>(data)[prop.pos] = parseColorModif(iEntry, widget);
            return true;
        case Type::SizeRelative:
            DIAG(if (fEntry > iEntry + 1) return false);
            ss = entryAsStrSize(iEntry, false);
            reinterpret_cast<SizeRelative*>(data)[prop.pos] = ss;
            return true;
        case Type::Coord:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<int*>(data)[prop.pos] = parseCoord(iEntry, widget);
            return true;
        case Type::FontIdx:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<int*>(data)[prop.pos] = getFont(entryAsStrId(iEntry, false));
            return true;
        case Type::ActionTable:
            return addAction(id, iEntry, fEntry, reinterpret_cast<int*>(data)[prop.pos], widget);
        case Type::Text:
            DIAG(if (fEntry > iEntry + 1) return false);
            ss = entryAsStrSize(iEntry, false);
            free(reinterpret_cast<char**>(data)[prop.pos]);
            reinterpret_cast<char**>(data)[prop.pos] = strndup(ss.first, ss.second);
            return true;
        case Type::TextPropOrStrId:
            DIAG(if (fEntry > iEntry + 1) return false);
            ss = entryAsStrSize(iEntry, true);
            if (*ss.first == '"')
                reinterpret_cast<TextPropOrStrId*>(data)[prop.pos] = strMng.add(ss.first + 1, ss.second - 2);
            else {
                const auto* propWidget(widget->getProp(entryId(iEntry)));
                if (!propWidget) return false;
                reinterpret_cast<TextPropOrStrId*>(data)[prop.pos] = propWidget->pos;
            }
            return true;
        default:
            DIAG(LOG("internal error, unhandled property type!"));
            return false;
        }
    }

    bool Application::addAction(Identifier actionId, int iEntry, int fEntry, int& actions, Widget* widget) {
        if (!actions || widget->isSharingActions()) {
            auto actionsPrev(actions);
            actions = int(actionTables.size());
            actionTables.resize(actionTables.size() + 1);
            widget->resetSharingActions();
            if (actionsPrev)
                actionTables[actions] = actionTables[actionsPrev];
        }
        auto& table(actionTables[actions]);
        int* tableEntry;
        switch (actionId) { // get the table entry to add commands to
        case Identifier::onEnter:
        case Identifier::onTimeout:      tableEntry = &table.onEnter;        break;
        case Identifier::onLeave:        tableEntry = &table.onLeave;        break;
        case Identifier::onClick:        tableEntry = &table.onClick;        break;
        case Identifier::onRender:       tableEntry = &table.onRender;       break;
        case Identifier::onRenderActive: tableEntry = &table.onRenderActive; break;
        default: DIAG(LOG("unknown action")); return false;
        }
        return addActionCommands(iEntry, fEntry, *tableEntry, widget);
    }

    bool Application::addActionCommands(int iEntry, int fEntry, int& tableEntry, Widget* widget) {
        if (*tree[iEntry].pos == '[') iEntry++;
        bool ok(true);
        tableEntry = int(actionCommands.size());
        while (iEntry < fEntry) {
            const auto& entry(tree[iEntry++]);
            auto command(entry.asId(tree, strMng));
            int next(entry.next ? entry.next : fEntry);

            const Type* params(nullptr);
            switch (command) {
            case Identifier::log:             params = logParams; break;
            case Identifier::toggleVisible:   params = toggleVisibleParams; break;
            case Identifier::beginPath:       params = beginPathParams; break;
            case Identifier::moveto:          params = movetoParams; break;
            case Identifier::lineto:          params = linetoParams; break;
            case Identifier::bezierto:        params = beziertoParams; break;
            case Identifier::closePath:       params = closePathParams; break;
            case Identifier::roundedRect:     params = roundedRectParams; break;
            case Identifier::fillColor:       params = fillColorParams; break;
            case Identifier::fillVertGrad:    params = fillVertGradParams; break;
            case Identifier::fill:            params = fillParams; break;
            case Identifier::strokeWidth:     params = strokeWidthParams; break;
            case Identifier::strokeColor:     params = strokeColorParams; break;
            case Identifier::stroke:          params = strokeParams; break;
            case Identifier::font:            params = fontParams; break;
            case Identifier::text:
            case Identifier::textLeft:        params = textParams; break;
            case Identifier::translateCenter: params = translateCenterParams; break;
            case Identifier::scale100:        params = scale100Params; break;
            case Identifier::resetTransform:  params = resetTransformParams; break;
            case Identifier::set:             params = setParams; break;
            case Identifier::query:           params = queryParams; break;
            default:
                DIAG(
                    auto ss(entry.asStrSize(tree, true));
                    LOG("unknown command: {%.*s}", ss.second, ss.first));
            }
            if (params) ok &= addCommandGeneric(command, iEntry, next, params, widget);
            iEntry = next;
        }
        actionCommands.push_back(int(Identifier::CLast));
        return ok;
    }

    bool Application::addCommandGeneric(Identifier name, int iEntry, int fEntry, const Type* params, Widget* widget) {
        DIAG(
            if (params[fEntry - iEntry] != Type::LastType) {
                LOG("%s command got %d parameters", strMng.get(name), fEntry - iEntry);
                return false;
            });
        actionCommands.push_back(int(name));
        Property prop;
        prop.all = 0;
        prop.size = 4;
        while (*params != Type::LastType) {
            int value;
            prop.type = *params++;
            int fTplPropSave(fTplProp);
            bool templateReplaced(replaceProperty(iEntry, iTplProp, fTplProp));
            if (!setProp(prop, Identifier::InvalidId, &value, iEntry, iEntry + 1, widget))
                DIAG(LOG("error in command argument"));
            iTplProp = replaceBackProperty(templateReplaced, iEntry, iTplProp);
            fTplProp = fTplPropSave;
            iEntry++;
            actionCommands.push_back(value);
        }
        return true;
    }

    bool Application::parseId(Widget* widget, const char*& str, const Property*& prop) const {
        if (isalpha(*str)) {
            const auto* start(str++);
            while (isalnum(*str)) ++str;
            auto id(strMng.search(start, str - start));
            if (id.valid()) return (prop = widget->getProp(Identifier(id.getId())));
            return false; // error
        }
        prop = nullptr;
        return true; // ok
    }

    int Application::parseCoord(int iEntry, Widget* widget) const {
        DIAG(bool bad(false));
        int out(0);
        auto param(entry(iEntry).pos);
        const Property* prop(nullptr);
        if (!parseId(widget, param, prop)) DIAG(bad = true);
        if (prop) {
            if (prop->size == 2 && prop->type == Type::Int16) out |= prop->pos << 16;
            else DIAG(bad = true);
        } else
            out |= 0xffff0000;
        if (*param == '+' || *param == '-' || isdigit(*param)) {
            float num(strtof(param, nullptr));
            out |= int(num*4) & 0xffff;
        }
        DIAG(
            if (bad) {
                auto ss(entryAsStrSize(iEntry, true));
                LOG("unrecognized coord string: %.*s", ss.second, ss.first);
            });
        return out;
    }

    RGBAref Application::parseColorModif(int iEntry, Widget* widget) const {
        auto param(entry(iEntry).pos);
        if (*param == '"') return RGBAref(param); // normal color, no reference
        // referenced color
        const Property* prop;
        if (!parseId(widget, param, prop) DIAG(|| !prop || prop->size != 4 || prop->type != Type::Color)) {
            DIAG(
                auto ss(entryAsStrSize(iEntry, true));
                LOG("unrecognized color string: %.*s", ss.second, ss.first));
            return RGBAref();
        } else {
            auto ref(prop->pos);
            float value(100);
            if (*param == '%') value = strtof(param + 1, nullptr);
            return RGBAref(ref, value);
        }
    }

    int Application::getFont(StringId str) {
        for (auto& font: fonts)
            if (font.first == str) return &font - fonts.data();
        // add font
        fonts.push_back(make_pair(str, nullptr));
        new RequestXHR(*this, RequestXHR::TypeFont, str, strMng.get(str));
        return fonts.size() - 1;
    }

    bool Application::executeNoCheck(int commandList, Widget* w) {
        bool cont(true), executed(false);
        V2s pos;
        float value;
        auto& render(ctx.getRender());
        while (cont) {
            auto* const command(&actionCommands[commandList]);
            switch (Identifier(*command)) {
            case Identifier::log:
                LOG("%s", strMng.get(StringId(command[1])));
                commandList += 2;
                break;
            case Identifier::toggleVisible:
                executed |= executeToggleVisible(StringId(command[1]));
                commandList += 2;
                break;
            case Identifier::beginPath:
                render.beginPath();
                ++commandList;
                break;
            case Identifier::moveto:
                render.moveto(getCoord(command[1], w), getCoord(command[2], w));
                commandList += 3;
                break;
            case Identifier::lineto:
                render.lineto(getCoord(command[1], w), getCoord(command[2], w));
                commandList += 3;
                break;
            case Identifier::bezierto:
                render.bezierto(getCoord(command[1], w), getCoord(command[2], w),
                                getCoord(command[3], w), getCoord(command[4], w),
                                getCoord(command[5], w), getCoord(command[6], w));
                commandList += 7;
                break;
            case Identifier::closePath:
                render.closePath();
                ++commandList;
                break;
            case Identifier::roundedRect:
                render.roundedRect(getCoord(command[1], w), getCoord(command[2], w), getCoord(command[3], w),
                                   getCoord(command[4], w), getCoord(command[5], w));
                commandList += 6;
                break;
            case Identifier::fillColor:
                render.fillColor(RGBAref(command[1]).get(w));
                commandList += 2;
                break;
            case Identifier::fillVertGrad:
                render.fillVertGrad(getCoord(command[1], w), getCoord(command[2], w), RGBAref(command[3]).get(w), RGBAref(command[4]).get(w));
                commandList += 5;
                break;
            case Identifier::fill:
                render.fill();
                ++commandList;
                break;
            case Identifier::strokeWidth:
                render.strokeWidth(*(float*)&command[1]);
                commandList += 2;
                break;
            case Identifier::strokeColor:
                render.strokeColor(RGBAref(command[1]).get(w));
                commandList += 2;
                break;
            case Identifier::stroke:
                render.stroke();
                ++commandList;
                break;
            case Identifier::font:
                if (command[1] < int(fonts.size()) && fonts[command[1]].second) {
                    render.font(command[1]);
                    fontValid = true;
                }
                render.fontSize(*(float*)&command[2]);
                commandList += 3;
                break;
            case Identifier::text:
                if (fontValid) {
                    render.textAlign(NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
                    pos = w->curPos + (w->curSize >> 1);
                    render.text(pos.x + getCoord(command[1], w), pos.y + getCoord(command[2], w),
                                reinterpret_cast<TextPropOrStrId*>(&command[3])->get(w, strMng));
                }
                commandList += 4;
                break;
            case Identifier::textLeft:
                if (fontValid) {
                    render.textAlign(NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
                    render.text(w->curPos.x + getCoord(command[1], w), w->curPos.y + (w->curSize.y >> 1) + getCoord(command[2], w),
                                reinterpret_cast<TextPropOrStrId*>(&command[3])->get(w, strMng));
                }
                commandList += 4;
                break;
            case Identifier::translateCenter:
                render.translate(w->curSize.x >> 1, w->curSize.y >> 1);
                ++commandList;
                break;
            case Identifier::scale100:
                value = getCoord(command[1], w) * 0.01;
                render.scale(value, value);
                commandList += 2;
                break;
            case Identifier::resetTransform:
                render.resetTransform();
                ++commandList;
                break;
            case Identifier::set:
                executed |= executeSet(StringId(command[1]), Identifier(command[2]), StringId(command[3]), w);
                commandList += 4;
                break;
            case Identifier::query:
                executed |= executeQuery(StringId(command[1]), StringId(command[2]));
                commandList += 3;
                break;
            case Identifier::CLast:
                cont = false;
                break;
            default:
                DIAG(
                    LOG("internal: executing command error: %d %d %zu", *command, commandList, actionCommands.size());
                    for (auto* command = &actionCommands[commandList]; command < &actionCommands.back(); command++)
                        LOG("%08x", *command));
                cont = false;
                break;
            }
        }
        return executed;
    }

    bool Application::executeToggleVisible(StringId widgetId) {
        auto len(getWidgetRange(widgetId));
        const char* idSearch(strMng.get(widgetId));
        bool toggled(false);
        for (auto& widget: widgets) {
            if (!memcmp(strMng.get(widget.first), idSearch, len)) {
                widget.second->toggleVisible();
                toggled = true;
            }
        }
        return toggled;
    }

    bool Application::executeSet(StringId widgetId, Identifier prop, StringId value, Widget* w) {
        bool set(false);
        auto iEntry(tree.getTemporalEntry(strMng.get(value)));
        if (widgetId.getId() == int(Identifier::self)) {
            // change own property
            if (!setProp(prop, w, iEntry, iEntry + 1))
                DIAG(LOG("error setting property"));
            set = true;
        } else {
            // change in the tree
            auto len(getWidgetRange(widgetId));
            const char* idSearch(strMng.get(widgetId));
            for (auto& widget: widgets) {
                if (!memcmp(strMng.get(widget.first), idSearch, len)) {
                    if (!setProp(prop, widget.second, iEntry, iEntry + 1))
                        DIAG(LOG("error setting property"));
                    set = true;
                }
            }
        }
        return set;
    }

    bool Application::executeQuery(StringId query, StringId widgetId) {
        // execute only in visible widgets
        auto it(widgets.find(widgetId));
        DIAG(
            if (it == widgets.end())
                LOG("internal: cannot find widget %s to send query", strMng.get(widgetId)));
        if (it->second->visible)
            new RequestXHR(*this, RequestXHR::TypeTemplate, widgetId, strMng.get(query));
        return true;
    }

    int Application::getWidgetRange(StringId widgetId) const {
        const char* id(strMng.get(widgetId));
        int len(strlen(id));
        if (len && id[len - 1] == '*') return len - 1;
        return len + 1;
    }

    DIAG(
        void Application::dump() const {
            if (root) {
                LOG("Application tree");
                root->dump(strMng);
            }
            LOG("Registered widgets:");
            for (const auto& widget: widgets)
                LOG("  %s", strMng.get(widget.first));
        });

}
