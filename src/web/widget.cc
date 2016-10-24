/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget.h"

namespace webui {

    void Widget::dump(int level) const {
        LOG("%*s%s: -", level * 2, "", id.c_str());
        for (const auto* child: children) child->dump(level + 1);
    }

}
