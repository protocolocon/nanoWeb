/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "reserved_words.h"
#include "compatibility.h"
#include "string_manager.h"
#include <cassert>

#define add(x)                                  \
    strMng.add(#x, constlen(#x) - 1);           \
    assert(strMng.search(#x).getId() ==         \
           int(Identifier::x))

namespace webui {

    void addReservedWords(StringManager& strMng) {
        add(Application);
        add(LayoutHor);
        add(Button);
        add(WLast);

        add(id);
        add(width);
        add(height);
        add(color);
        add(onEnter);
        add(ALast);

        add(log);
        add(toggle);
        add(CLast);
    }

}
