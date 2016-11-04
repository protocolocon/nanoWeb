/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "vector.h"
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
        bool setProp(const Properties& props, Identifier id, void* data, int iEntry, int fEntry);

        // actions
        struct ActionTable {
            inline ActionTable(): onEnter(0), onLeave(0), onClick(0), onRender(0) { }
            int onEnter;
            int onLeave;
            int onClick;
            int onRender;
        };
        struct ActionRenderContext {
            ActionRenderContext(): zero(0) { }
            V2s pos, size;
            short zero;
        };
        bool addAction(Identifier actionId, int iEntry, int fEntry, int& actions); // add action to widget
        inline const ActionTable& getActionTable(int actions) const { return actionTables[actions]; }
        inline ActionRenderContext& getActionRenderContext() { return actionRenderContext; }
        inline bool execute(int commandList) { return commandList ? executeNoCheck(commandList) : false; } // true if commands executed
        bool executeNoCheck(int commandList);

        // debug
        void dump() const;

    private:
        Context& ctx;
        RequestXHR xhr;
        MLParser parser;
        StringManager strMng;

        // layout
        bool layoutStable;

        // commands
        enum CommandId {
            CommandLog,
            CommandToggleVisible,
            CommandBeginPath,
            CommandRoundedRect,
            CommandFillColor,
            CommandFillVertGrad,
            CommandStrokeWidth,
            CommandStrokeColor,
            CommandStroke,
            CommandLast
        };
        std::vector<ActionTable> actionTables;
        std::vector<int> actionCommands;
        ActionRenderContext actionRenderContext;

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
        enum class ParamType: int { StrId, Coord, Float, Color };
        bool addActionCommands(int iEntry, int fEntry, int& tableEntry);
        bool addCommandGeneric(Identifier name, CommandId command, int iEntry, int fEntry, const std::vector<ParamType>& params);
        bool addCommandToggle(int iEntry);
        bool executeToggleVisible(StringId widgetId); // returns true if something toggled

        int parseCoord(int iEntry) const;
        inline float getCoord(int c) const { return float((((short*)&actionRenderContext)[c >> 16] << 2) + short(c & 0xffff)) * 0.25f; }
    };

}
