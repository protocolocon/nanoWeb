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
    class WidgetTemplate;

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
        bool addAction(Identifier actionId, int iAction, Widget* widget); // add action to widget

        // fonts
        int getFont(StringId str);

        // XHR
        bool onLoad(RequestXHR* xhr);
        void onError(RequestXHR* xhr);

        // template
        bool updateTemplate(WidgetTemplate* widget);
        inline MLParser& getTemplateParser() { return tpl; }
        bool startTemplate(int& iTpl, int& fTpl);
        bool endTemplate();

        // debug
        DIAG(void dump() const);
        inline auto* getRoot() { return root; }
        inline auto& getWidgets() { return widgets; }
        /*TODO*/bool executeToggleVisible(StringId widgetId); // returns true if something toggled
        /*TODO*/bool executeQuery(StringId query, StringId widgetId);
        Widget* createWidget(Identifier id, Widget* parent, int objectSize = 0);

    private:
        MLParser tree, tpl;    // application tree description and template data
        int iTpl, fTpl;
        bool startedTpl;

        // layout
        bool layoutStable;

        std::vector<ActionTable> actionTables;

        // widget tree and registration
        Widget* root;
        std::unordered_map<StringId, Widget*, StringId> widgets;

        // fonts
        std::vector<std::pair<StringId, char*>> fonts;

        Widget* initializeConstruct();
        Widget* initializeConstruct(Widget* widget, int iEntry, int fEntry, bool& define, bool recurse);

        // widget factory and registration
        bool isWidget(Identifier id) const;
        bool registerWidget(Widget* widget);
        int getWidgetRange(StringId widgetId) const;
        Widget* createType(Widget* widget, Identifier typeId, int iEntry, int fEntry);

        // actions
        bool checkActions();

        // timers
        bool refreshTimers(); // returns true on command execution

        bool parseId(Widget* widget, const char*& str, const Property*& prop) const;
        static inline float getCoord(int c, Widget* widget) {
            int x(short(c & 0xffff));
            if (c >= 0) x += ((short*)widget)[c >> 16] << 2;
            return float(x) * 0.25f;
        }
    };

}
