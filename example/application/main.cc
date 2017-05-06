/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "server.h"
#include <iostream>

using namespace std;
using namespace server;

namespace {

    class Example {
    public:
        Example(Application& app);
        ~Example();

        // hooks
        static Example* create(Application& app) { return new Example(app); }
        static void destroy(void* example) { delete static_cast<Example*>(example); }
    };

    Example::Example(Application& app) {
        cout << "create example app session" << endl;
        app.text(0, 16, "0123456789 ABCDEFG example áñᙥさ", 100, 100);
    }

    Example::~Example() {
        cout << "destroy example app session" << endl;
    }

}

int main(int argc, char* argv[]) {
    Server server;
    if (server.addFont("lucida_sans_unicode.ttf") != 0) cout << "expecting font id to be 0" << endl;
    server.onCreateApp(&Example::create);
    server.onDestroyApp(&Example::destroy);
    server.run(3000);
    return 0;
}
