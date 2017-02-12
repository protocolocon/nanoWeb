/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "types.h"
#include "vector.h"
#include "type_widget.h"
#include "compatibility.h"
#include "string_manager.h"
#include "reserved_words.h"
#include <vector>
#include <string>

namespace webui {

    struct Property;

    class Widget {
    public:
        Widget(Widget* parent);
        virtual ~Widget();

        // type related
        inline Identifier type() const { return typeWidget->type; }
        inline int typeSize() const { return typeWidget->size; }
        static TypeWidget& getType();

        // hierarchy
        inline void addChild(Widget* child) { children.push_back(child); }
        inline Widget* getParent() { return parent; }
        inline auto& getChildren() { return children; }

        // polymorphic interface
        virtual Identifier baseType() const { return Identifier::Widget; }
        virtual void render(int alphaMult);
        virtual bool input(); // returns true if actions were executed (affecting application)
        virtual bool layout(const Box4f& box); // returns true if stable

        // get a property or null
        const Property* getProp(StringId id) const;

        // copy properties from provided widget
        void copyFrom(const Widget* widget);

        // update widget status
        bool update(); // returns true if actions were executed (affecting application)

        // getters
        inline const StringId getId() const { return id; }
        inline V2s getSizeTarget(V2s s) const { return V2s(size[0].get(s.x), size[1].get(s.y)); }
        inline bool isVisible() const { return visible; }
        inline bool isSharingActions() const { return sharedActions; }
        inline void resetSharingActions() { sharedActions = 0; }

        // setters
        inline void setId(StringId id_) { id = id_; }
        inline void setVisible(bool v) { visible = v; }
        inline void toggleVisible() { visible ^= 1; }

        // utils
        void translate(V2f t);
        bool animeAlpha();

        // debug
        DIAG(void dump(int level = 0, bool props = false) const);

    public:
        // dynamic part
        Box4f box;          // bounding box: absolute position

        // static part
        SizeRelative size[2];
        TypeWidget* typeWidget;
        Widget* parent;
        StringId id;
        std::vector<Widget*> children;
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
