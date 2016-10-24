/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include <string>
#include <unordered_map>

namespace webui {

    class Widget;
    class Context;
    class MLParser;

    class Application {
    public:
        Application(Context& ctx);
        ~Application();
        void refresh();
        void clear();

        // debug
        void dump() const;

    private:
        bool init;
        Context& ctx;
        RequestXHR xhr;

        // widget tree and registration
        Widget* root;
        std::unordered_map<std::string, Widget*> widgets;

        void initialize();
        Widget* initializeConstruct(const MLParser& parser);
        bool initializeConstruct(const MLParser& parser, Widget* widget, int iEntry, int fEntry);

        // widget factory and registration
        Widget* createWidget(const std::string& name, Widget* parent);
        bool registerWidget(Widget* widget);
    };

}
