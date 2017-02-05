/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <vector>

namespace webui {

    class LinearArrangement {
    public:
        struct Elem {
            bool resizable;
            short pos, posTarget;
        };

    public:
        inline LinearArrangement(Elem* elems, int initialPos);
        inline void add(int sizeCurrent, int sizeTarget, bool resizable);
        bool calculate(int sizeAvailable); // returns true if stable
        inline int get(int idx) const { return elems[idx].pos; }
        void dump() const;

    private:
        Elem* elems;
        int nElems;
    };


    LinearArrangement::LinearArrangement(Elem* elems, int initialPos): elems(elems), nElems(1) {
        elems[0].resizable = false;
        elems[0].pos = elems[0].posTarget = initialPos;
    }

    void LinearArrangement::add(int sizeCurrent, int sizeTarget, bool resizable) {
        auto& b(elems[nElems - 1]);
        auto& e(elems[nElems++]);
        e.resizable = resizable;
        e.pos = b.pos + sizeCurrent;
        e.posTarget = b.posTarget + sizeTarget;
    }

}
