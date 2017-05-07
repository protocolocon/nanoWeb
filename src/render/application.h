/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "protocol.h"
#include "compatibility.h"
#include <vector>

namespace render {

    class Application: protected prot::ApplicationBase {
    public:
        Application();
        void refresh();

    private:
        int delayProgress;
        std::vector<uint8_t> delay;

        void doDelay();
        bool doDelayIfFailed(bool delayed, bool ok);
        bool execute(bool delayed);     // execute command in transfer, returns false if failed
        bool commandText(); // false if failed
        void commandFont();

        void missingFont(int font);
    };

}
