/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "render.h"
#include "application.h"

namespace webui {

    class Context {
    public:
        Context();

        void mainIteration();

        inline void forceRender() { renderForced = true; }

        inline Render& getRender() { return render; }

    private:
        Render render;
        Application app;
        bool renderForced;
    };

}
