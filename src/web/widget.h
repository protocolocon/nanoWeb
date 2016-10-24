/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

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

        virtual void render(Context& ctx) { }

        // getters
        inline const std::string& getId() const { return id; }

        // setters
        inline void setId(const std::string& id_) { id = id_; }

        // debug
        void dump(int level = 0) const;

    private:
        std::string id;
        Widget* parent;
        std::vector<Widget*> children;
    };

}
