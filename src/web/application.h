/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "vector.h"
#include "ml_parser.h"
#include "properties.h"
#include "compatibility.h"
#include "string_manager.h"
#include "reserved_words.h"
#include <string>
#include <unordered_map>

namespace webui {

    class Widget;
    class Context;
    class MLParser;
    struct Property;
    class Properties;

    class Application {
    public:
        Application(Context& ctx);
        ~Application();

        // wipes-out application definition
        void clear();

        // resize
        void resize(int width, int height);

        // check if application status needs update
        void refresh();
        bool update();

        // render
        void render();

        // get parsed entry
        inline const MLParser::Entry& entry(int idx) const { return parser[idx]; }
        inline std::pair<const char*, int> entryAsStrSize(int idx) const { return parser[idx].asStrSize(parser); }
        inline Identifier entryId(int idx) const { return parser[idx].asId(parser, strMng); }
        inline StringId entryAsStrId(int idx) { return parser[idx].asStrId(parser, strMng); }

        // set generic properties
        bool setProp(Identifier id, Widget* widget, int iEntry, int fEntry);
        bool setProp(const Property& prop, Identifier id, void* data, int iEntry, int fEntry, Widget* widget);

        // actions
        struct ActionTable {
            inline ActionTable(): onEnter(0), onLeave(0), onClick(0), onRender(0) { }
            int onEnter;
            int onLeave;
            int onClick;
            int onRender;
        };
        bool addAction(Identifier actionId, int iEntry, int fEntry, int& actions, Widget* widget); // add action to widget
        inline const ActionTable& getActionTable(int actions) const { return actionTables[actions]; }
        inline bool execute(int commandList, Widget* widget) { return commandList ? executeNoCheck(commandList, widget) : false; } // true if commands executed
        bool executeNoCheck(int commandList, Widget* widget);

        // debug
        void dump() const;

    private:
        Context& ctx;
        RequestXHR xhr;
        MLParser parser;
        StringManager strMng;

        // layout
        bool layoutStable;

        std::vector<ActionTable> actionTables;
        std::vector<int> actionCommands;

        // widget tree and registration
        Widget* root;
        std::unordered_map<StringId, Widget*, StringId> widgets;

        void initialize();
        Widget* initializeConstruct(const MLParser& parser);
        bool initializeConstruct(const MLParser& parser, Widget* widget, int iEntry, int fEntry);
        void refreshNetwork();

        // widget factory and registration
        Widget* createWidget(Identifier id, Widget* parent);
        bool registerWidget(Widget* widget);

        // actions
        bool addActionCommands(int iEntry, int fEntry, int& tableEntry, Widget* widget);
        bool addCommandGeneric(Identifier name, int iEntry, int fEntry, const Type* params, Widget* widget);
        bool addCommandToggle(int iEntry);
        bool executeToggleVisible(StringId widgetId); // returns true if something toggled

        int parseCoord(int iEntry, Widget* widget) const;
        static inline float getCoord(int c, Widget* widget) {
            int x(short(c & 0xffff));
            if (c >= 0) x += ((short*)widget)[c >> 16] << 2;
            return float(x) * 0.25f;
        }
    };

}
