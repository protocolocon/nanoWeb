/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include <string>
#include <unordered_map>

struct JSON;

namespace webui {

    class Widget;

    class Application {
    public:
        Application();
        void refresh();

    private:
        void initialize();

        bool init;
        RequestXHR xhr;

        Widget* root;
        std::unordered_map<std::string, Widget*> widgets;
    };

}
