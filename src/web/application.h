/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "ml_parser.h"
#include "compatibility.h"
#include "string_manager.h"
#include "reserved_words.h"
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

        // resize
        void resize(int width, int height);

        // render
        void render();

        // get parsed entry
        inline const MLParser::Entry& entry(int idx) const { return parser[idx]; }
        inline std::pair<const char*, int> entryAsStrSize(int idx) const { return parser[idx].asStrSize(parser); }
        inline Identifier entryId(int idx) const { return parser[idx].asId(parser, strMng); }
        inline StringId entryAsStrId(int idx) { return parser[idx].asStrId(parser, strMng); }

        // debug
        void dump() const;

    private:
        bool init;
        Context& ctx;
        RequestXHR xhr;
        MLParser parser;
        StringManager strMng;

        // widget tree and registration
        Widget* root;
        std::unordered_map<StringId, Widget*, StringId> widgets;

        void initialize();
        Widget* initializeConstruct(const MLParser& parser);
        bool initializeConstruct(const MLParser& parser, Widget* widget, int iEntry, int fEntry);

        // widget factory and registration
        Widget* createWidget(Identifier id, Widget* parent);
        bool registerWidget(Widget* widget);
    };

}
