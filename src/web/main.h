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

        void mainIteration();

        void resize(int width, int height);

        inline void forceRender() { renderForced = true; }

        inline Render& getRender() { return render; }

        inline int getTimeUs() const { return timeUs; }
        inline int getTimeDiffUs() const { return timeDiffUs; }
        inline int getTimeRatio() const { return timeRatio; }
        inline int getTime1MRatio() const { return time1MRatio; }
        inline void resetRatio() { timeRatio = 0x10000; time1MRatio = 0; }
        template <typename T>
        inline bool getCloser(T& x, int target) const {
            int xx((int(x) * time1MRatio + target * timeRatio) >> 16);
            if (x == xx && xx != target) xx = xx < target ? xx + 1 : xx - 1;
            x = xx;
            return xx == target;
        }
        inline LinearArrangement& getLinearArrangement() { return linearArrangement; }

    private:
        Render render;
        Application app;
        bool renderForced;
        int timeUs;
        int timeDiffUs;
        int timeRatio;
        int time1MRatio;

        LinearArrangement linearArrangement;

        void updateTime();
    };

}
