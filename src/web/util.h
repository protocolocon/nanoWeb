/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <vector>

namespace webui {

    class Context;

    class LinearArrangement {
    public:
        inline void init(int initialPos, int nElements);
        inline void add(int sizeCurrent, int sizeTarget, bool resizable);
        bool calculate(Context& ctx, int sizeAvailable); // returns true if stable
        inline int get(int idx) const { return elems[idx].pos; }

    private:
        struct Elem {
            Elem() { }
            Elem(bool r, int p, int t): resizable(r), pos(p), posTarget(t) { }

            bool resizable;
            int pos, posTarget;
        };

        std::vector<Elem> elems;
    };


    void LinearArrangement::init(int initialPos, int nElements) {
        elems.clear();
        elems.reserve(nElements + 1);
        elems.push_back(Elem(false, initialPos, initialPos));
    }

    void LinearArrangement::add(int sizeCurrent, int sizeTarget, bool resizable) {
        auto& b(elems.back());
        elems.push_back(Elem(resizable, b.pos + sizeCurrent, b.posTarget + sizeTarget));
    }

}
