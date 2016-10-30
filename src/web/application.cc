/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "main.h"
#include "input.h"
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

    Application::Application(Context& ctx): ctx(ctx), layoutStable(false), actionTables(1), actionCommands(1, CommandLast) {
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
            ctx.getRender().beginFrame();
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
                    if (!widget->set(*this, key, iEntry + 1, next)) {
                        auto ss(entryVal.asStrSize(parser));
                        LOG("warning: unknwon attribute %s with value %.*s", strMng.get(StringId(int(key))), ss.second, ss.first);
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

    bool Application::addAction(Identifier actionId, int iEntry, int fEntry, int& actions) {
        if (!actions) {
            actions = int(actionTables.size());
            actionTables.resize(actionTables.size() + 1);
        }
        auto& table(actionTables[actions]);
        int* tableEntry;
        switch (actionId) { // get the table entry to add commands to
        case Identifier::onEnter: tableEntry = &table.onEnter; break;
        case Identifier::onLeave: tableEntry = &table.onLeave; break;
        case Identifier::onClick: tableEntry = &table.onClick; break;
        default: LOG("unknown action"); return false;
        }
        return addActionCommands(iEntry, fEntry, *tableEntry);
    }

    bool Application::addActionCommands(int iEntry, int fEntry, int& tableEntry) {
        if (*parser[iEntry].pos == '[') iEntry++;
        bool ok(true);
        pair<const char*, int> ss;
        tableEntry = int(actionCommands.size());
        while (iEntry < fEntry) {
            const auto& entry(parser[iEntry]);
            auto command(entry.asId(parser, strMng));
            int next(entry.next ? entry.next : fEntry);
            switch (command) {
            case Identifier::log:           ok &= addCommandGenericStrId(command, CommandLog, iEntry + 1, next); break;
            case Identifier::toggleVisible: ok &= addCommandGenericStrId(command, CommandToggleVisible, iEntry + 1, next); break;
            default:
                ss = entry.asStrSize(parser);
                LOG("unknown command: {%.*s}", ss.second, ss.first);
                return false;
            }
            iEntry = next;
        }
        actionCommands.push_back(CommandLast);
        return ok;
    }

    bool Application::addCommandGenericStrId(Identifier name, CommandId command, int iEntry, int fEntry) {
        if (fEntry - iEntry != 1) {
            LOG("%s command requires one parameter", strMng.get(StringId(int(name))));
            return false;
        }
        actionCommands.push_back(command);
        actionCommands.push_back(entryAsStrId(iEntry).getId());
        return true;
    }

    bool Application::executeInner(int commandList) {
        bool cont(true), executed(false);
        auto* command(&actionCommands[commandList]);
        while (cont) {
            switch (*command) {
            case CommandLog:
                LOG("%s", strMng.get(StringId(*++command)));
                ++command;
                break;
            case CommandToggleVisible:
                executed |= executeToggleVisible(StringId(*++command));
                ++command;
                break;
            case CommandLast:
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
