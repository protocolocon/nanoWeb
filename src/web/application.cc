/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "main.h"
#include "widget.h"
#include "ml_parser.h"
#include "widget_button.h"
#include "reserved_words.h"
#include "widget_layout_hor.h"
#include "widget_application.h"
#include <cstdlib>
#include <cassert>
#include <algorithm>

using namespace std;

namespace webui {

    Application::Application(Context& ctx): init(false), ctx(ctx) {
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
            MLParser parser;
            if (!parser.parse(xhr.getData(), xhr.getNData()) || !(root = initializeConstruct(parser))) {
                xhr.makeCString();
                LOG("cannot parse: %s", xhr.getData());
                clear();
            } else
                resize(ctx.getRender().getWidth(), ctx.getRender().getHeight());

            //parser.dumpTree();
            xhr.clear();
            ctx.forceRender();
            dump();
        }
    }

    Widget* Application::initializeConstruct(const MLParser& parser) {
        if (parser.size() > 1 && parser[0].next == 1 && parser[1].next == 0 && *parser[1].pos == '{') {
            auto* widget(createWidget(parser[0].getId(parser, strMng), nullptr));
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
            auto key(entryKey.getId(parser, strMng));
            if (key == Identifier::InvalidId) LOG("error: invalid identifier {%s}", entryKey.getSimpleValue(parser).c_str());
            int next(entryVal.next ? entryVal.next : fEntry);
            if (key < Identifier::WidgetLast && (widgetChild = createWidget(key, widget/*parent*/))) {
                // child widget
                if (!initializeConstruct(parser, widgetChild, iEntry + 2, next) || !registerWidget(widgetChild)) {
                    delete widgetChild;
                    return false;
                }
                widget->addChild(widgetChild);
            } else {
                auto value(entryVal.getSimpleValue(parser));
                if (!widget->set(key, strMng, value))
                    LOG("warning: unknwon attribute %s with value %s", strMng.get(StringId(int(key))), value.c_str());
            }
            iEntry = next;
        }
        return true;
    }

    Widget* Application::createWidget(Identifier id, Widget* parent) {
        /**/ if (id == Identifier::Application) return new WidgetApplication(parent);
        else if (id == Identifier::Button)      return new WidgetButton(parent);
        else if (id == Identifier::LayoutHor)   return new WidgetLayoutHor(parent);
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
            LOG("repeated widget id: %s", strMng.get(id));
            return false;
        }
        widgets[id] = widget;
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
