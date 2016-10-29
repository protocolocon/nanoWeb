/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "main.h"
#include "widget.h"
#include "widget_button.h"
#include "reserved_words.h"
#include "widget_layout_hor.h"
#include "widget_application.h"
#include <cstdlib>
#include <cassert>
#include <algorithm>

using namespace std;

namespace webui {

    Application::Application(Context& ctx): init(false), ctx(ctx), actionTables(1) {
        addReservedWords(strMng);
    }

    Application::~Application() {
        clear();
    }

    void Application::refresh() {
        if (!init) initialize();
    }

    void Application::clear() {
        root = nullptr;
        for (auto& widget: widgets)
            delete widget.second;
        widgets.clear();
    }

    void Application::resize(int width, int height) {
        if (root)
            root->layout(V2s(0, 0), V2s(ctx.getRender().getWidth(), ctx.getRender().getHeight()), -1.0f);
    }

    void Application::render() {
        if (root) {
            ctx.getRender().beginFrame();
            root->render(ctx);
            ctx.getRender().endFrame();
        }
    }

    void Application::initialize() {
        if (xhr.getStatus() == RequestXHR::Empty) xhr.query("application.ml");
        else if (xhr.getStatus() == RequestXHR::Ready) {
            init = true;

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
            if (key == Identifier::InvalidId) {
                auto ss(entryKey.asStrSize(parser));
                LOG("error: invalid identifier {%.*s}", ss.second, ss.first);
            }
            int next(entryVal.next ? entryVal.next : fEntry);
            if (key < Identifier::WLast && (widgetChild = createWidget(key, widget/*parent*/))) {
                // child widget
                if (!initializeConstruct(parser, widgetChild, iEntry + 2, next) || !registerWidget(widgetChild)) {
                    delete widgetChild;
                    return false;
                }
                widget->addChild(widgetChild);
            } else {
                if (!widget->set(*this, key, iEntry + 1)) {
                    auto ss(entryVal.asStrSize(parser));
                    LOG("warning: unknwon attribute %s with value %.*s", strMng.get(StringId(int(key))), ss.second, ss.first);
                }
            }
            iEntry = next;
        }
        return true;
    }

    Widget* Application::createWidget(Identifier id, Widget* parent) {
        switch (id) {
        case Identifier::Application: return new WidgetApplication(parent);
        case Identifier::Button:      return new WidgetButton(parent);
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

    bool Application::addAction(Identifier actionId, int iEntry, int& actions) {
        if (!actions) {
            actions = int(actionTables.size());
            actionTables.resize(actionTables.size() + 1);
        }
        auto& table(actionTables[actions]);
        int* tableEntry;
        switch (actionId) {
        case Identifier::onEnter: tableEntry = &table.onEnter; break;
        default: LOG("unknown action"); return false;
        }
        return addActionCommands(iEntry, *tableEntry);
    }

    bool Application::addActionCommands(int iEntry, int& tableEntry) {
        if (*parser[iEntry].pos == '[') iEntry++;
        bool ok(true);
        pair<const char*, int> ss;
        tableEntry = int(actionCommands.size());
        while (iEntry) {
            const auto& entry(parser[iEntry]);
            auto command(entry.asId(parser, strMng));
            // check if parameters are empty
            int paramEntry(iEntry + 1);
            if (parser[paramEntry].next == paramEntry + 1 || (!parser[paramEntry].next && parser.getLevelEnd(paramEntry) == paramEntry))
                paramEntry = 0;
            switch (command) {
            case Identifier::log:    ok &= addCommandLog(paramEntry); break;
            case Identifier::toggle: ok &= addCommandToggle(paramEntry); break;
            default:
                ss = entry.asStrSize(parser);
                LOG("unknown command: {%.*s}", ss.second, ss.first);
                return false;
            }
            iEntry = entry.next;
        }
        actionCommands.push_back(CommandLast);
        return ok;
    }

    bool Application::addCommandLog(int iEntry) {
        if (!iEntry || parser[iEntry].next) {
            LOG("log command requires one parameter");
            return false;
        }
        actionCommands.push_back(CommandLog);
        actionCommands.push_back(entryAsStrId(iEntry).getId());
        return true;
    }

    bool Application::addCommandToggle(int iEntry) {
        if (!iEntry || parser[iEntry].next) {
            LOG("toggle command requires one parameter");
            return false;
        }
        actionCommands.push_back(CommandToggle);
        // TODO
        return true;
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
