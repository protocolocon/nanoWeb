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
#include "widget_layout_hor.h"
#include "widget_application.h"
#include <cstdlib>
#include <cassert>
#include <algorithm>

using namespace std;

namespace webui {

    Application::Application(Context& ctx): init(false), ctx(ctx) {
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
            auto* widget(createWidget(parser[0].getSimpleValue(parser), nullptr));
            if (initializeConstruct(parser, widget, 2, parser.size()) && registerWidget(widget))
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
            auto key(entryKey.getSimpleValue(parser));
            int next(entryVal.next ? entryVal.next : fEntry);
            if ((widgetChild = createWidget(key, widget/*parent*/))) {
                // child widget
                if (!initializeConstruct(parser, widgetChild, iEntry + 2, next) || !registerWidget(widgetChild)) {
                    delete widgetChild;
                    return false;
                }
                widget->addChild(widgetChild);
            } else {
                auto value(entryVal.getSimpleValue(parser));
                if (!widget->set(key, value))
                    LOG("warning: unknwon attribute %s with value %s", key.c_str(), value.c_str());
            }
            iEntry = next;
        }
        return true;
    }

    Widget* Application::createWidget(const std::string& name, Widget* parent) {
        /**/ if (name == "Application") return new WidgetApplication(parent);
        else if (name == "Button")      return new WidgetButton(parent);
        else if (name == "LayoutHor")   return new WidgetLayoutHor(parent);
        return nullptr;
    }

    bool Application::registerWidget(Widget* widget) {
        if (widget->getId().empty()) {
            // set an internal id
            static int id(0);
            char buffer[16];
            int nBuffer(snprintf(buffer, sizeof(buffer), "@%x", id++));
            widget->setId(string(buffer, nBuffer));
        }
        const auto& id(widget->getId());
        if (widgets.count(id)) {
            LOG("repeated widget id: %s", id.c_str());
            return false;
        }
        widgets[id] = widget;
        return true;
    }

    void Application::dump() const {
        if (root) {
            LOG("Application tree");
            root->dump();
        }
        LOG("Registered widgets:");
        for (const auto& widget: widgets)
            LOG("  %s", widget.first.c_str());
    }

}
