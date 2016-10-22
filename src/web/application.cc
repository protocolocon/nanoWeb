/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "json.h"
#include "compatibility.h"

using namespace std;

namespace webui {

    Application::Application(): status(false) {
    }

    bool Application::initialized() const {
        return status;
    }

    bool Application::init(const RequestXHR& data) {
        JSON val[100];
        int n = jsonparse(data.getData(), val, sizeof(val) / sizeof(JSON));
        cout << n << endl;

        status = true;
        return false;
    }

}
