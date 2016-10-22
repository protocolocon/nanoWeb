/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "json.h"

using namespace std;

namespace webui {

    Application::Application(): init(false) {
    }

    void Application::refresh() {
        if (!init) {
            if (xhr.getStatus() == RequestXHR::Empty) xhr.query("application");
            else if (xhr.getStatus() == RequestXHR::Ready) {
                init = true;

                auto data(xhr.getData());
                auto nData(xhr.getNData());
                if (nData && data[nData-1] == '\n') data[nData-1] = 0; // zero-terminated

                JSON val[100];
                int n = jsonparse(data, val, sizeof(val) / sizeof(JSON));
                LOG("JSON: %d", n);
            }
        }
    }

}
