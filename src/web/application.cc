/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "main.h"
#include "input.h"
#include "widget.h"
#include "properties.h"
#include "reserved_words.h"
#include "widget_layout_hor.h"
#include "widget_application.h"
#include <cstdlib>
#include <cassert>
#include <algorithm>

using namespace std;
using namespace webui;

namespace {

    Properties actionTableProperties = {
        { Identifier::onEnter,   PROP(Application::ActionTable, onEnter,   ActionEntry,  4, 0, 0) },
        { Identifier::onLeave,   PROP(Application::ActionTable, onLeave,   ActionEntry,  4, 0, 0) },
        { Identifier::onClick,   PROP(Application::ActionTable, onClick,   ActionEntry,  4, 0, 0) },
        { Identifier::onRender,  PROP(Application::ActionTable, onRender,  ActionEntry,  4, 0, 0) },
    };

    const Type logParams[] =           { Type::StrId, Type::LastType };
    const Type toggleVisibleParams[] = { Type::StrId, Type::LastType };
    const Type beginPathParams[] =     { Type::LastType };
    const Type roundedRectParams[] =   { Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::LastType };
    const Type fillColorParams[] =     { Type::Color, Type::LastType };
    const Type fillVertGradParams[] =  { Type::Coord, Type::Coord, Type::Color, Type::Color, Type::LastType };
    const Type strokeWidthParams[] =   { Type::Float, Type::LastType };
    const Type strokeColorParams[] =   { Type::Color, Type::LastType };
    const Type strokeParams[] =        { Type::LastType };
    Properties commandParameterProperties = {
        { Identifier::log,           PROPDIFF(logParams,           logParams) },
        { Identifier::toggleVisible, PROPDIFF(toggleVisibleParams, logParams) },
        { Identifier::beginPath,     PROPDIFF(beginPathParams,     logParams) },
        { Identifier::roundedRect,   PROPDIFF(roundedRectParams,   logParams) },
        { Identifier::fillColor,     PROPDIFF(fillColorParams,     logParams) },
        { Identifier::fillVertGrad,  PROPDIFF(fillVertGradParams,  logParams) },
        { Identifier::strokeWidth,   PROPDIFF(strokeWidthParams,   logParams) },
        { Identifier::strokeColor,   PROPDIFF(strokeColorParams,   logParams) },
        { Identifier::stroke,        PROPDIFF(strokeParams,        logParams) },
    };

}

namespace webui {

    Application::Application(Context& ctx): ctx(ctx), layoutStable(false), actionTables(1), actionCommands(1, int(Identifier::CLast)), root(nullptr) {
        addReservedWords(strMng);
        initialize();
    }

    Application::~Application() {
        clear();
    }

    void Application::refresh() {
        refreshNetwork();
        if (root && (Input::refresh(ctx.getRender().getWin()) || !layoutStable)) {
            layoutStable = root->layout(ctx, V2s(0, 0), V2s(ctx.getRender().getWidth(), ctx.getRender().getHeight()));
            ctx.forceRender();
        }
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

    void Application::initialize() {
        assert(xhr.getStatus() == RequestXHR::Empty);
        xhr.query("application.ml");
    }

    void Application::refreshNetwork() {
        if (xhr.getStatus() == RequestXHR::Ready) {
            // TODO: different queries
            assert(!root);
            if (!parser.parse(xhr.getData(), xhr.getNData()) || !(root = initializeConstruct(parser))) {
                xhr.makeCString();
                LOG("cannot parse: %s", xhr.getData());
                clear();
            } else
                resize(ctx.getRender().getWidth(), ctx.getRender().getHeight());

            parser.dumpTree();
            parser.clear();
            xhr.clear();
            ctx.forceRender();
            dump();
        }
    }

    Widget* Application::initializeConstruct(const MLParser& parser) {
        if (parser.size() > 1 && parser[0].next == 1 && parser[1].next == 0 && *parser[1].pos == '{') {
            auto* widget(createWidget(parser[0].asId(parser, strMng), nullptr));
            if (widget && initializeConstruct(parser, widget, 2, parser.size()) && registerWidget(widget))
                return widget;
            delete widget;
        }
        return nullptr;
    }

    bool Application::initializeConstruct(const MLParser& parser, Widget* widget, int iEntry, int fEntry) {
        Widget* widgetChild;
        while (iEntry < fEntry) {
            // key : value
            const auto& entryKey(parser[iEntry]);
            assert(iEntry + 1 < fEntry && entryKey.next == iEntry + 1);
            const auto& entryVal(parser[iEntry + 1]);
            // key and next after value
            auto key(entryKey.asId(parser, strMng));
            int next(entryVal.next ? entryVal.next : fEntry);
            if (key == Identifier::InvalidId) {
                auto ss(entryKey.asStrSize(parser));
                LOG("error: invalid identifier {%.*s}", ss.second, ss.first);
            } else {
                if (key < Identifier::WLast && (widgetChild = createWidget(key, widget/*parent*/))) {
                    // child widget
                    if (!initializeConstruct(parser, widgetChild, iEntry + 2, next) || !registerWidget(widgetChild)) {
                        delete widgetChild;
                        return false;
                    }
                    widget->addChild(widgetChild);
                } else {
                    if (!setProp(widget->getProps(), key, widget, iEntry + 1, next)) {
                        auto ss(entryVal.asStrSize(parser));
                        LOG("warning: unknown attribute %s with value %.*s", strMng.get(StringId(int(key))), ss.second, ss.first);
                    }
                }
            }
            iEntry = next;
        }
        return true;
    }

    Widget* Application::createWidget(Identifier id, Widget* parent) {
        switch (id) {
        case Identifier::Application: return new WidgetApplication(parent);
        case Identifier::Widget:      return new Widget(parent);
        case Identifier::LayoutHor:   return new WidgetLayoutHor(parent);
        default:                      return nullptr;
        }
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
            LOG("repeated widget id: %s", strMng.get(id));
            return false;
        }
        widgets[id] = widget;
        return true;
    }

    bool Application::setProp(const Properties& props, Identifier id, void* data, int iEntry, int fEntry) {
        auto it(props.find(id));
        if (it == props.end()) return false;
        if (!setProp(it->second, id, data, iEntry, fEntry)) {
            auto ss(entryAsStrSize(iEntry));
            LOG("%s was expecting the type %s, received %.*s (%d parameters) and failed",
                strMng.get(id), toString(it->second.type), ss.second, ss.first, fEntry - iEntry);
            return false;
        }
        return true;
    }

    bool Application::setProp(const Property& prop, Identifier id, void* data, int iEntry, int fEntry) {
        pair<const char*, int> ss;
        switch (prop.type) {
        case Type::StrId:
            if (fEntry > iEntry + 1) return false;
            reinterpret_cast<StringId*>(data)[prop.pos] = entryAsStrId(iEntry);
            return true;
        case Type::Float:
            if (fEntry > iEntry + 1) return false;
            reinterpret_cast<float*>(data)[prop.pos] = strtof(parser[iEntry].pos, nullptr);
            return true;
        case Type::Color:
            if (fEntry > iEntry + 1) return false;
            reinterpret_cast<RGBA*>(data)[prop.pos] = entry(iEntry).pos;
            return true;
        case Type::SizeRelative:
            if (fEntry > iEntry + 1) return false;
            ss = entryAsStrSize(iEntry);
            reinterpret_cast<SizeRelative*>(data)[prop.pos] = ss;
            return true;
        case Type::Coord:
            if (fEntry > iEntry + 1) return false;
            reinterpret_cast<int*>(data)[prop.pos] = parseCoord(iEntry);
            return true;
        case Type::ActionTable:
            return addAction(id, iEntry, fEntry, reinterpret_cast<int*>(data)[prop.pos]);
        case Type::ActionEntry:
            return addActionCommands(iEntry, fEntry, reinterpret_cast<int*>(data)[prop.pos]);
        default:
            LOG("internal error, unhandled property type!");
            return false;
        }
    }

    bool Application::addAction(Identifier actionId, int iEntry, int fEntry, int& actions) {
        if (!actions) {
            actions = int(actionTables.size());
            actionTables.resize(actionTables.size() + 1);
        }
        auto& table(actionTables[actions]);
        if (!setProp(actionTableProperties, actionId, &table, iEntry, fEntry)) {
            LOG("unknown action %s or invalid parameters", strMng.get(actionId));
            return false;
        }
        return true;
    }

    bool Application::addActionCommands(int iEntry, int fEntry, int& tableEntry) {
        if (*parser[iEntry].pos == '[') iEntry++;
        bool ok(true);
        tableEntry = int(actionCommands.size());
        while (iEntry < fEntry) {
            const auto& entry(parser[iEntry++]);
            auto command(entry.asId(parser, strMng));
            int next(entry.next ? entry.next : fEntry);

            auto it(commandParameterProperties.find(command));
            if (it == commandParameterProperties.end()) {
                auto ss(entry.asStrSize(parser));
                LOG("unknown command: {%.*s}", ss.second, ss.first);
                return false;
            } else {
                auto* params(logParams + int(int16_t(it->second.pos)));
                ok &= addCommandGeneric(command, iEntry, next, params);
            }
            iEntry = next;
        }
        actionCommands.push_back(int(Identifier::CLast));
        return ok;
    }

    bool Application::addCommandGeneric(Identifier name, int iEntry, int fEntry, const Type* params) {
        if (params[fEntry - iEntry] != Type::LastType) {
            LOG("%s command got %d parameters", strMng.get(name), fEntry - iEntry);
            return false;
        }
        actionCommands.push_back(int(name));
        Property prop;
        prop.all = 0;
        prop.size = 4;
        while (*params != Type::LastType) {
            int value;
            prop.type = *params++;
            if (!setProp(prop, Identifier::InvalidId, &value, iEntry, iEntry + 1)) LOG("error in command argument");
            iEntry++;
            actionCommands.push_back(value);
        }
        return true;
    }

    int Application::parseCoord(int iEntry) const {
        bool bad(false);
        int out(0);
        auto param(entryAsStrSize(iEntry));
        const auto* end(param.first + param.second);
        if (isalpha(*param.first)) {
            // identifier
            /**/ if (*param.first == 'x') { out |= 0 << 16; param.first++; }
            else if (*param.first == 'y') { out |= 1 << 16; param.first++; }
            else if (*param.first == 'w') { out |= 2 << 16; param.first++; }
            else if (*param.first == 'h') { out |= 3 << 16; param.first++; }
            else bad = true;
            if (isalnum(*param.first)) bad = true;
        } else
            out |= 4 << 16;
        bool neg(false);
        if (*param.first == '+' || (neg = (*param.first == '-'))) param.first++;
        while (param.first < end && isspace(*param.first)) param.first++;
        if (param.first < end) {
            float num(strtof(param.first, nullptr));
            if (neg) num = -num;
            out |= int(num*4) & 0xffff;
        }
        if (bad) {
            param = entryAsStrSize(iEntry);
            LOG("unrecognized coord string: %.*s", param.second, param.first);
        }
        return out;
    }

    bool Application::executeNoCheck(int commandList) {
        bool cont(true), executed(false);
        auto* command(&actionCommands[commandList]);
        auto& render(ctx.getRender());
        while (cont) {
            switch (Identifier(*command)) {
            case Identifier::log:
                LOG("%s", strMng.get(StringId(*++command)));
                ++command;
                break;
            case Identifier::toggleVisible:
                executed |= executeToggleVisible(StringId(*++command));
                ++command;
                break;
            case Identifier::beginPath:
                render.beginPath();
                ++command;
                break;
            case Identifier::roundedRect:
                render.roundedRect(getCoord(command[1]), getCoord(command[2]), getCoord(command[3]), getCoord(command[4]), getCoord(command[5]));
                command += 6;
                break;
            case Identifier::fillColor:
                render.fillColor(RGBA(*++command));
                ++command;
                break;
            case Identifier::fillVertGrad:
                render.fillVertGrad(getCoord(command[1]), getCoord(command[2]), RGBA(command[3]), RGBA(command[4]));
                command += 5;
                break;
            case Identifier::strokeWidth:
                render.strokeWidth(*(float*)++command);
                ++command;
                break;
            case Identifier::strokeColor:
                render.strokeColor(RGBA(*++command));
                ++command;
                break;
            case Identifier::stroke:
                render.stroke();
                ++command;
                break;
            case Identifier::CLast:
                cont = false;
                break;
            default:
                LOG("internal: executing command error: %d %d %zu", *command, commandList, actionCommands.size());
                for (command = &actionCommands[commandList]; command < &actionCommands.back(); command++)
                    LOG("%08x", *command);
                cont = false;
                break;
            }
        }
        return executed;
    }

    bool Application::executeToggleVisible(StringId widgetId_) {
        const char* widgetId(strMng.get(widgetId_));
        int len(strlen(widgetId));
        bool toggled(false);
        if (len) {
            LOG("toggle visibility of %s", widgetId);
            StringId first, last;
            if (widgetId[len - 1] == '*') {
                // match prefix
                abort(); // TODO
            } else {
                // exact match
                first = last = widgetId_;
            }
            for (auto& widget: widgets) {
                if (widget.first >= first && widget.first <= last) {
                    widget.second->toggleVisible();
                    toggled = true;
                }
            }
        }
        return toggled;
    }

    void Application::dump() const {
        if (root) {
            LOG("Application tree");
            root->dump(strMng);
        }
        LOG("Registered widgets:");
        for (const auto& widget: widgets)
            LOG("  %s", strMng.get(widget.first));
    }

}
