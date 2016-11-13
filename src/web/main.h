/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "util.h"
#include "render.h"
#include "application.h"

namespace webui {

    class Context {
    public:
        Context();
        ~Context();

        void initialize() { app.initialize(); }

        void mainIteration();

        void resize(int width, int height);

        inline void forceRender() { renderForced = true; }

        inline Render& getRender() { return render; }
        inline Application& getApplication() { return app; }

        inline int getTimeMs() const { return timeMs; }
        inline int getTimeDiffMs() const { return timeDiffMs; }
        inline int getTimeRatio() const { return timeRatio; }
        inline int getTime1MRatio() const { return time1MRatio; }
        inline void resetRatio() { timeRatio = 0x10000; time1MRatio = 0; }
        template <typename T>
        inline bool getCloser(T& x, int target) const {
            int xx((int(x) * timeRatio + target * time1MRatio) >> 16);
            if (x == xx && xx != target) xx = xx < target ? xx + 1 : xx - 1;
            x = xx;
            return xx == target;
        }

    private:
        Render render;
        Application app;
        bool renderForced;
        int timeMs;
        int timeDiffMs;
        int timeRatio;
        int time1MRatio;

        void updateTime();
    };

}
