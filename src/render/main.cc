/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "context.h"
#include "compatibility.h"

using namespace std;
using namespace render;

namespace {

    inline void mainIteration() {
        ctx.mainIteration();
    }

    extern "C" {
        void javascriptCanvasResize(int width, int height) {
            ctx.resize(width, height);
        }
    }

}

int main(int argc, char* argv[]) {
    // main loop
    setCommandLine(argc, argv);
    ctx.initialize(DIAG(true, true));
    setMainLoop(mainIteration);
    return 0;
}
