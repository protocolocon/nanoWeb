/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "vector.h"
#include "compatibility.h"
#include <vector>
#include <string>

namespace webui {

    class Context;

    class Widget {
    public:
        inline Widget(Widget* parent = nullptr): parent(parent) { }
        virtual ~Widget() { }

        // hierarchy
        inline void addChild(Widget* child) { children.push_back(child); }
        inline Widget* getParent() { return parent; }
        inline auto& getChildren() { return children; }

        // polymorphic interface
        virtual void render(Context& ctx);
        virtual bool set(const std::string& param, const std::string& value); // returns true if set

        // getters
        inline const std::string& getId() const { return id; }

        // setters
        inline void setId(const std::string& id_) { id = id_; }
        inline void setWidth(int w) { size[0] = w; }
        inline void setHeight(int h) { size[1] = h; }

        // debug
        void dump(int level = 0) const;

    protected:
        Widget* parent;
        V2i pos;
        V2i size;
        std::string id;
        std::vector<Widget*> children;
    };

}
