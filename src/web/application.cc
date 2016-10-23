/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "widget.h"
#include "ml_parser.h"
#include <algorithm>

using namespace std;

namespace webui {

    Application::Application(): init(false) {
    }

    void Application::refresh() {
        if (!init) initialize();
    }

    void Application::initialize() {
        if (xhr.getStatus() == RequestXHR::Empty) xhr.query("application.ml");
        else if (xhr.getStatus() == RequestXHR::Ready) {
            init = true;

            MLParser parser;
            parser.parse(xhr.getData(), xhr.getNData());
            //parser.dump();
            parser.dumpTree();
            // TODO: use description

            xhr.clear();
        }
    }

}
