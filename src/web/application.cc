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

    const Type logParams[] =           { Type::StrId, Type::LastType };
    const Type toggleVisibleParams[] = { Type::StrId, Type::LastType };
    const Type beginPathParams[] =     { Type::LastType };
    const Type roundedRectParams[] =   { Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::LastType };
    const Type fillColorParams[] =     { Type::ColorModif, Type::LastType };
    const Type fillVertGradParams[] =  { Type::Coord, Type::Coord, Type::ColorModif, Type::ColorModif, Type::LastType };
    const Type fillParams[] =          { Type::LastType };
    const Type strokeWidthParams[] =   { Type::Float, Type::LastType };
    const Type strokeColorParams[] =   { Type::ColorModif, Type::LastType };
    const Type strokeParams[] =        { Type::LastType };
    const Type setParams[] =           { Type::StrId, Type::Id, Type::Str, Type::LastType };
    const Type fontParams[] =          { Type::FontIdx, Type::Float, Type::LastType };
    const Type textParams[] =          { Type::Coord, Type::Coord, Type::TextPropOrStrId, Type::LastType };
    const Type queryParams[] =         { Type::StrId, Type::StrId, Type::LastType };

}

namespace webui {

    Application::Application(Context& ctx): ctx(ctx), layoutStable(false), actionTables(1), actionCommands(1, int(Identifier::CLast)), root(nullptr),
                                            fontValid(false) {
    }

    Application::~Application() {
        clear();
        for (auto& font: fonts)
            free(font.second);
    }

    void Application::initialize() {
        addReservedWords(strMng);
        auto id(strMng.add("application.ml"));
        new RequestXHR(*this, RequestXHR::TypeApplication, id, strMng.get(id));
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
        if (xhr->getType() == RequestXHR::TypeApplication) {
            assert(!root);
            if (!parser.parse(xhr->getData(), xhr->getNData()) || !(root = initializeConstruct(parser))) {
                DIAG(
                    xhr->makeCString();
                    LOG("cannot parse: %s", xhr->getData()));
                clear();
            } else
                resize(ctx.getRender().getWidth(), ctx.getRender().getHeight());

            parser.clear();
            ctx.forceRender();
            DIAG(
                //parser.dumpTree();
                dump());
        } else if (xhr->getType() == RequestXHR::TypeFont) {
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
                LOG("internal error with font indices: %s", strMng.get(id));
            ctx.forceRender();
        } else {
            xhr->makeCString();
            LOG("unknown query: %s %s", strMng.get(xhr->getId()), xhr->getData());
        }
        delete xhr;
    }

    void Application::onError(RequestXHR* xhr) {
        LOG("lost query: %s", strMng.get(xhr->getId()));
        delete xhr;
    }

    Widget* Application::initializeConstruct(const MLParser& parser) {
        if (parser.size() > 1 && parser[0].next == 1 && parser[1].next == 0 && *parser[1].pos == '{') {
            bool define;
            auto* widget(createWidget(parser[0].asId(parser, strMng), nullptr));
            if (widget && initializeConstruct(parser, widget, 2, parser.size(), define, true) && registerWidget(widget))
                return widget;
            delete widget;
        }
        return nullptr;
    }

    bool Application::initializeConstruct(const MLParser& parser, Widget* widget, int iEntry, int fEntry, bool& define, bool recurse) {
        define = false;
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
                DIAG(
                    auto ss(entryKey.asStrSize(parser, true));
                    LOG("error: invalid identifier {%.*s}", ss.second, ss.first);
                    parser.error(entryKey.pos, "=>", entryKey.line));
            } else {
                if ((widgetChild = createWidget(key, widget/*parent*/))) {
                    if (recurse) {
                        // child widget
                        bool defineChild(false);
                        bool isTemplate(key == Identifier::Template);
                        if (!initializeConstruct(parser, widgetChild, iEntry + 2, next, defineChild, !isTemplate) || !registerWidget(widgetChild)) {
                            delete widgetChild;
                            return false;
                        }
                        if (!defineChild) widget->addChild(widgetChild);
                        if (isTemplate) {
                            auto* tpl(reinterpret_cast<WidgetTemplate*>(widgetChild));
                            parser.copyTo(tpl->getParser(), iEntry + 2, next);
                        }
                    } else
                        delete widgetChild;
                } else {
                    if (key == Identifier::define) {
                        key = Identifier::id;
                        define = true;
                    }
                    if (!setProp(key, widget, iEntry + 1, next)) {
                        DIAG(
                            auto ss(entryVal.asStrSize(parser, true));
                            LOG("warning: unknown attribute %s with value %.*s", strMng.get(StringId(int(key))), ss.second, ss.first);
                            parser.error(entryKey.pos, "=>", entryKey.line));
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
        if (!prop) return false;
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
        case Type::Uint8:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<uint8_t*>(data)[prop.pos] = atoi(parser[iEntry].pos);
            return true;
        case Type::Int32:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<int*>(data)[prop.pos] = atoi(parser[iEntry].pos);
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
            reinterpret_cast<float*>(data)[prop.pos] = strtof(parser[iEntry].pos, nullptr);
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
        if (*parser[iEntry].pos == '[') iEntry++;
        bool ok(true);
        tableEntry = int(actionCommands.size());
        while (iEntry < fEntry) {
            const auto& entry(parser[iEntry++]);
            auto command(entry.asId(parser, strMng));
            int next(entry.next ? entry.next : fEntry);

            const Type* params(nullptr);
            switch (command) {
            case Identifier::log:           params = logParams; break;
            case Identifier::toggleVisible: params = toggleVisibleParams; break;
            case Identifier::beginPath:     params = beginPathParams; break;
            case Identifier::roundedRect:   params = roundedRectParams; break;
            case Identifier::fillColor:     params = fillColorParams; break;
            case Identifier::fillVertGrad:  params = fillVertGradParams; break;
            case Identifier::fill:          params = fillParams; break;
            case Identifier::strokeWidth:   params = strokeWidthParams; break;
            case Identifier::strokeColor:   params = strokeColorParams; break;
            case Identifier::stroke:        params = strokeParams; break;
            case Identifier::set:           params = setParams; break;
            case Identifier::font:          params = fontParams; break;
            case Identifier::text:          params = textParams; break;
            case Identifier::query:         params = queryParams; break;
            default:
                DIAG(
                    auto ss(entry.asStrSize(parser, true));
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
            if (!setProp(prop, Identifier::InvalidId, &value, iEntry, iEntry + 1, widget))
                DIAG(LOG("error in command argument"));
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
                render.roundedRect(getCoord(command[1], w), getCoord(command[2], w), getCoord(command[3], w),
                                   getCoord(command[4], w), getCoord(command[5], w));
                command += 6;
                break;
            case Identifier::fillColor:
                render.fillColor(RGBAref(*++command).get(w));
                ++command;
                break;
            case Identifier::fillVertGrad:
                render.fillVertGrad(getCoord(command[1], w), getCoord(command[2], w), RGBAref(command[3]).get(w), RGBAref(command[4]).get(w));
                command += 5;
                break;
            case Identifier::fill:
                render.fill();
                ++command;
                break;
            case Identifier::strokeWidth:
                render.strokeWidth(*(float*)++command);
                ++command;
                break;
            case Identifier::strokeColor:
                render.strokeColor(RGBAref(*++command).get(w));
                ++command;
                break;
            case Identifier::stroke:
                render.stroke();
                ++command;
                break;
            case Identifier::font:
                if (command[1] < int(fonts.size()) && fonts[command[1]].second) {
                    render.font(command[1]);
                    fontValid = true;
                }
                render.fontSize(*(float*)&command[2]);
                command += 3;
                break;
            case Identifier::text:
                if (fontValid) {
                    render.textAlign(NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
                    pos = w->curPos + (w->curSize >> 1);
                    render.text(pos.x + getCoord(command[1], w), pos.y + getCoord(command[2], w),
                                reinterpret_cast<TextPropOrStrId*>(&command[3])->get(w, strMng));
                }
                command += 4;
                break;
            case Identifier::set:
                executed |= executeSet(StringId(command[1]), Identifier(command[2]), StringId(command[3]), w);
                command += 4;
                break;
            case Identifier::query:
                executed |= executeQuery(StringId(command[1]), StringId(command[2]));
                command += 3;
                break;
            case Identifier::CLast:
                cont = false;
                break;
            default:
                DIAG(
                    LOG("internal: executing command error: %d %d %zu", *command, commandList, actionCommands.size());
                    for (command = &actionCommands[commandList]; command < &actionCommands.back(); command++)
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
        auto iEntry(parser.getTemporalEntry(strMng.get(value)));
        if (widgetId.getId() == int(Identifier::self)) {
            // change own property
            setProp(prop, w, iEntry, iEntry + 1);
            set = true;
        } else {
            // change in the tree
            auto len(getWidgetRange(widgetId));
            const char* idSearch(strMng.get(widgetId));
            for (auto& widget: widgets) {
                if (!memcmp(strMng.get(widget.first), idSearch, len)) {
                    setProp(prop, widget.second, iEntry, iEntry + 1);
                    set = true;
                }
            }
        }
        return set;
    }

    bool Application::executeQuery(StringId query, StringId widgetId) {
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
