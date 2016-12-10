/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "types.h"
#include "vector.h"
#include "compatibility.h"
#include "string_manager.h"
#include "reserved_words.h"
#include <vector>
#include <string>

namespace webui {

    class Context;
    struct Property;
    class Properties;
    class Application;

    class Widget {
    public:
        Widget(Widget* parent);
        virtual ~Widget();

        // hierarchy
        inline void addChild(Widget* child) { children.push_back(child); }
        inline Widget* getParent() { return parent; }
        inline auto& getChildren() { return children; }

        // polymorphic interface
        virtual Identifier type() const { return Identifier::Widget; }
        virtual void render(Context& ctx, int alphaMult);
        virtual bool input(Application& app); // returns true if actions were executed (affecting application)
        virtual bool layout(Context& ctx, V2s pos, V2s size); // returns true if stable
        virtual const Properties& getProps() const;

        // get a property in this widget (polymorphism) or in Widget or null
        const Property* getProp(Identifier id) const;

        // copy properties from provided widget
        void copyFrom(const Widget* widget);

        // update widget status
        bool update(Application& app); // returns true if actions were executed (affecting application)

        // animations
        bool animeAlpha(Context& ctx);

        // getters
        inline const StringId getId() const { return id; }
        inline V2s getSizeTarget(V2s s) const { return V2s(size[0].get(s.x), size[1].get(s.y)); }
        inline bool isVisible() const { return visible; }
        inline RGBA getBackground() const { return background; }
        inline RGBA getForeground() const { return foreground; }
        inline bool isSharingActions() const { return sharedActions; }
        inline void resetSharingActions() { sharedActions = 0; }

        // setters
        inline void setId(StringId id_) { id = id_; }
        inline void setVisible(bool v) { visible = v; }
        inline void toggleVisible() { visible ^= 1; }

        // utils
        void translate(V2s t);

        // debug
        DIAG(void dump(const StringManager& strMng, int level = 0) const);

    public:
        // dynamic part
        V2s curPos;         // absolute position
        V2s curSize;

        // static part
        Widget* parent;
        char* text;
        StringId id;
        std::vector<Widget*> children;
        SizeRelative size[2];
        RGBA background, foreground;
        union {
            uint32_t all;
            struct {
                uint8_t byte0;
                uint8_t byte1;
                uint8_t byte2;
                uint8_t byte3;
            };
            struct {
                uint8_t visible:1;        // if the widget and its children are visible
                uint8_t inside:1;         // cursor is inside widget
                uint8_t sharedActions:1;  // indicates that is sharing the action table
                uint8_t canFocus:1;       // if can receive events
                uint8_t active:1;         // if widget is in it's active status (like a pressed button)
                uint8_t draggable:1;      // if widget is draggable
                uint8_t reserved:2;
                uint8_t zoom;
                uint8_t alpha;
                uint8_t reserved2;
            };
        };
        int actions;
    };

}
