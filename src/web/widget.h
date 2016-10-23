/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <vector>

namespace webui {

    class Context;

    class Widget {
    public:
        inline Widget(Widget* parent = nullptr): parent(parent) { }
        virtual ~Widget();

        // hierarchy
        inline void addChild(Widget* child) { children.push_back(child); }
        inline Widget* getParent() { return parent; }
        inline auto& getChildren() { return children; }

        virtual void render(Context& ctx);

    private:
        Widget* parent;
        std::vector<Widget*> children;
    };

}
