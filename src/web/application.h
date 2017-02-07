/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "types.h"
#include "vector.h"
#include "ml_parser.h"
#include "type_widget.h"
#include "compatibility.h"
#include "string_manager.h"
#include "reserved_words.h"
#include <string>
#include <unordered_map>

namespace webui {

    class Widget;
    class MLParser;
    struct Property;

    class Application {
    public:
        Application();
        DIAG(~Application());
        void initialize();

        // wipes-out application definition
        DIAG(void clear());

        // resize
        void resize(int width, int height);

        // check if application status needs update
        void refresh();
        bool update();

        // render
        void render();

        // add/set generic properties
        bool setProp(Identifier id, Widget* widget, int iEntry, int fEntry);
        bool setProp(const Property& prop, Identifier id, void* data, int iEntry, int fEntry, Widget* widget);

        // actions
        struct ActionTable {
            inline ActionTable(): onEnter(0), onLeave(0), onClick(0), onRender(0), onRenderActive(0) { }
            union {
                struct {
                    int onEnter;
                    int onLeave;
                    int onClick;
                    int onRender;
                    int onRenderActive;
                };
                int actions[5];
            };
            bool checked;
        };
        inline const ActionTable& getActionTable(int actions) const { return actionTables[actions]; }

        // fonts
        int getFont(StringId str);

        // XHR
        bool onLoad(RequestXHR* xhr);
        void onError(RequestXHR* xhr);

        // debug
        DIAG(void dump() const);
        inline auto* getRoot() { return root; }
        inline auto& getWidgets() { return widgets; }

    private:
        MLParser tree, tpl;    // application tree description and template data
        int iTpl, fTpl;

        // layout
        bool layoutStable;

        std::vector<ActionTable> actionTables;

        // widget tree and registration
        Widget* root;
        std::unordered_map<StringId, Widget*, StringId> widgets;

        // fonts
        std::vector<std::pair<StringId, char*>> fonts;
        bool fontValid;

        Widget* initializeConstruct();
        Widget* initializeConstruct(Widget* widget, int iEntry, int fEntry, bool& define, bool recurse);

        // widget factory and registration
        bool isWidget(Identifier id) const;
        Widget* createWidget(Identifier id, Widget* parent, int objectSize = 0);
        bool registerWidget(Widget* widget);
        int getWidgetRange(StringId widgetId) const;
        Widget* createType(Widget* widget, Identifier typeId, int iEntry, int fEntry);

        // templates in properties
        bool replaceProperty(int iEntry);
        void replaceBackProperty(bool templateReplaced, int iEntry);

        // actions
        bool checkActions();
        bool addAction(Identifier actionId, int iEntry, int fEntry, int& widgetActions, Widget* widget); // add action to widget
        bool addCommandGeneric(Identifier name, int iEntry, int fEntry, const Type* params, Widget* widget);
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
