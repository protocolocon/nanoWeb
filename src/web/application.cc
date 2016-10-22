/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "compatibility.h"

namespace webui {

    bool Application::initialized() const {
        return false;
    }

    bool Application::init(const RequestXHR& data) {
        return false;
    }

}
