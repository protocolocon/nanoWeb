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
        Example(ServerApp& app);
        ~Example();

        // hooks
        static Example* create(ServerApp& app) { return new Example(app); }
        static void destroy(void* example) { delete static_cast<Example*>(example); }
    };

    Example::Example(ServerApp& app) {
        cout << this << ": create session" << endl;
        app.onReceiveMessage([&](char* message, int size) {
                cout << this << ": " << string(message, size) << endl;
            });
        app.sendMessage("hello world", 11);
    }

    Example::~Example() {
        cout << this << ": destroy session" << endl;
    }

}

int main(int argc, char* argv[]) {
    Server server;
    server.onCreateApp(&Example::create);
    server.onDestroyApp(&Example::destroy);
    server.run(3000);
    return 0;
}
