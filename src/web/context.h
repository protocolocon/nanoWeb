/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "util.h"
#include "render.h"
#include "action.h"
#include "application.h"

namespace webui {

    class Context {
    public:
        Context();
        DIAG(~Context());

        void initialize(DIAG(bool initRender, bool requestAppDescription));

        void mainIteration();

        void resize(int width, int height);

        inline void forceRender() { renderForced = true; }

        inline int getTimeMs() const { return timeMs; }
        inline int getTimeDiffMs() const { return timeDiffMs; }
        inline int getTimeRatio() const { return timeRatio; }
        inline int getTime1MRatio() const { return time1MRatio; }
        inline void resetRatio() { timeRatio = 0x10000; time1MRatio = 0; }
        template <typename T>
        inline bool getCloser(T& xT, int target) const {
            int x(xT);
            int xx((x * timeRatio + target * time1MRatio) >> 16);
            if (x == xx && xx != target) xx = xx < target ? xx + 1 : xx - 1;
            xT = xx;
            return x == target;
        }

    public:
        // global context
        static Render render;
        static Actions actions;
        static Application app;
        static StringManager strMng;
        static Box4f renderVisibilityBox;
        static Cursor cursor;
        static Widget* hoverWidget;

    private:
        bool renderForced;
        int timeMs;
        int timeDiffMs;
        int timeRatio;
        int time1MRatio;

        void updateTime();
    };

    // global context
    extern Context ctx;

}
