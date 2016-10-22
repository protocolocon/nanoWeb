/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"

namespace webui {

    class Application {
    public:
        Application();
        void refresh();

    private:
        bool init;
        RequestXHR xhr;
    };

}
