/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "client_app.h"

using namespace render;

int main(int argc, char* argv[]) {
    ClientApp app(argc, argv);
    int fontRobotoRegular(-1);
    app.onInit([&]() {
            LOG("init");
            fontRobotoRegular = app.fontAdd("Roboto-Regular.ttf");
        });
    app.onConnected([&]() {
            app.sendMessage("connected", 9);
        });
    app.onReceiveMessage([&](char* message, int size) -> bool {
            LOG("received: %*s", size, message);
            app.sendMessage("hi back", 7);
            return true;
        });
    app.onReceiveResource([&]() {
            LOG("receive resource");
            if (app.fontCheck(fontRobotoRegular))
                app.text(fontRobotoRegular, 32, "hello world");
        });

    if (!app.run()) return 1;
    return 0;
}
