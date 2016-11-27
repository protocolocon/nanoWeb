/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "types.h"
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
        void initialize();

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
        inline const MLParser::Entry& entry(int idx) const { return tree[idx]; }
        inline std::pair<const char*, int> entryAsStrSize(int idx, bool quotes) const { return tree[idx].asStrSize(tree, quotes); }
        inline Identifier entryId(int idx) const { return tree[idx].asId(tree, strMng); }
        inline StringId entryAsStrId(int idx, bool quotes) { return tree[idx].asStrId(tree, strMng, quotes); }

        // set generic properties
        bool setProp(Identifier id, Widget* widget, int iEntry, int fEntry);
        bool setProp(const Property& prop, Identifier id, void* data, int iEntry, int fEntry, Widget* widget);

        // actions
        struct ActionTable {
            inline ActionTable(): onEnter(0), onLeave(0), onClick(0), onRender(0), onRenderActive(0) { }
            int onEnter;
            int onLeave;
            int onClick;
            int onRender;
            int onRenderActive;
        };
        bool addAction(Identifier actionId, int iEntry, int fEntry, int& actions, Widget* widget); // add action to widget
        inline const ActionTable& getActionTable(int actions) const { return actionTables[actions]; }
        inline bool execute(int commandList, Widget* widget) { return commandList ? executeNoCheck(commandList, widget) : false; } // true if commands executed
        bool executeNoCheck(int commandList, Widget* widget);

        // XHR
        void onLoad(RequestXHR* xhr);
        void onError(RequestXHR* xhr);

        // debug
        DIAG(void dump() const);
        inline auto& getStrMng() { return strMng; }

    private:
        Context& ctx;
        MLParser tree, tpl;    // application tree description and template data
        StringManager strMng;

        // layout
        bool layoutStable;

        std::vector<ActionTable> actionTables;
        std::vector<int> actionCommands;

        // widget tree and registration
        Widget* root;
        std::unordered_map<StringId, Widget*, StringId> widgets;

        // fonts
        std::vector<std::pair<StringId, char*>> fonts;
        bool fontValid;

        Widget* initializeConstruct();
        bool initializeConstruct(Widget* widget, int iEntry, int fEntry, int iTpl, int jTpl, bool& define, bool recurse);

        // widget factory and registration
        bool isWidget(Identifier id) const;
        Widget* createWidget(Identifier id, Widget* parent);
        bool registerWidget(Widget* widget);
        int getWidgetRange(StringId widgetId) const;

        // fonts
        int getFont(StringId str);

        // actions
        bool addActionCommands(int iEntry, int fEntry, int& tableEntry, Widget* widget);
        bool addCommandGeneric(Identifier name, int iEntry, int fEntry, const Type* params, Widget* widget);
        bool addCommandToggle(int iEntry);
        bool executeToggleVisible(StringId widgetId); // returns true if something toggled
        bool executeSet(StringId widgetId, Identifier prop, StringId value, Widget* widget);
        bool executeQuery(StringId query, StringId widgetId);

        // timers
        bool refreshTimers(); // returns true on command execution

        bool parseId(Widget* widget, const char*& str, const Property*& prop) const;
        int parseCoord(int iEntry, Widget* widget) const;
        RGBAref parseColorModif(int iEntry, Widget* widget) const;
        static inline float getCoord(int c, Widget* widget) {
            int x(short(c & 0xffff));
            if (c >= 0) x += ((short*)widget)[c >> 16] << 2;
            return float(x) * 0.25f;
        }
    };

}
