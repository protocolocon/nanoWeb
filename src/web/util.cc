/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "util.h"
#include "context.h"
#include "compatibility.h"

using namespace std;

namespace webui {

    template <typename T>
    bool LinearArrangement<T>::calculate(T sizeAvailable) {
        auto& f(elems[0]);
        auto& b(elems[nElems - 1]);
        T total(b.posTarget - f.posTarget);
        if (total > sizeAvailable) {
            T resizables(0);
            for (int i = nElems - 1; i > 0; i--) {
                elems[i].posTarget = elems[i].posTarget - elems[i-1].posTarget; // convert to sizes
                if (elems[i].resizable) resizables += elems[i].posTarget;
            }
            T ratio((sizeAvailable - total + resizables) / resizables);
            for (int i = 1; i < nElems; i++) {
                if (elems[i].resizable)
                    elems[i].posTarget = elems[i-1].posTarget + elems[i].posTarget * ratio + 0.5f; // convert to pos
                else
                    elems[i].posTarget = elems[i-1].posTarget + elems[i].posTarget; // convert to pos
            }
        }
        // converge positions
        bool stable(true);
        for (int i = 1; i < nElems; i++)
            stable &= ctx.getCloser(elems[i].pos, elems[i].posTarget);
        return stable;
    }

    template <typename T>
    void LinearArrangement<T>::dump() const {
        for (int i = 0; i < nElems; i++) {
            const auto& elem(elems[i]);
            LOG("LA: %d %.1f %.1f", elem.resizable, elem.pos, elem.posTarget);
        }
    }


    // explicit template instantiations
    template bool LinearArrangement<float>::calculate(float sizeAvailable);
    template void LinearArrangement<float>::dump() const;

}
