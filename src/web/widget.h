/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "vector.h"
#include "compatibility.h"
#include "string_manager.h"
#include "reserved_words.h"
#include <vector>
#include <string>

namespace webui {

    class Context;

    class Widget {
    public:
        inline Widget(Widget* parent = nullptr): parent(parent), size(0x100, 0x100), wRelative(1), hRelative(1) { }
        virtual ~Widget() { }

        // hierarchy
        inline void addChild(Widget* child) { children.push_back(child); }
        inline Widget* getParent() { return parent; }
        inline auto& getChildren() { return children; }

        // polymorphic interface
        virtual void render(Context& ctx);
        virtual bool layout(V2s pos, V2s size, float time); // returns true if stable
        virtual bool set(Identifier id, StringManager& strMng, const std::string& value); // returns true if set

        // getters
        inline const StringId getId() const { return id; }
        inline int getWidthTarget(int width)   const { return wRelative ? (width  * int(size.x)) >> 8 : size.x; }
        inline int getHeightTarget(int height) const { return hRelative ? (height * int(size.y)) >> 8 : size.y; }
        inline V2s getSizeTarget(V2s size)     const { return V2s(getWidthTarget(size.x), getHeightTarget(size.y)); }
        inline bool isWidthRelative()  const { return wRelative; }
        inline bool isHeightRelative() const { return hRelative; }

        // setters
        inline void setId(StringId id_) { id = id_; }
        inline void setWidth(int w) { size[0] = w; }
        inline void setHeight(int h) { size[1] = h; }

        // debug
        void dump(const StringManager& strMng, int level = 0) const;

    protected:
        // dynamic part
        V2s curPos;         // absolute position
        V2s curSize;

        // static part
        StringId id;
        Widget* parent;
        std::vector<Widget*> children;
        V2s size;
        uint8_t wRelative:1;
        uint8_t hRelative:1;
        uint8_t reserved:6;
        uint8_t zoom;
    };

}
