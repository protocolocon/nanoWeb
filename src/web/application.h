/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

namespace webui {

    class RequestXHR;

    class Application {
    public:
        bool initialized() const;
        bool init(const RequestXHR& data);
    };

}
