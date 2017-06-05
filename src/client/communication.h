/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "protocol.h"

namespace render {

    class ChunkedCommunication: prot::ChunkedCommunicationBase {
    public:
        ChunkedCommunication(): initialized(false) { }
        void refresh();

    private:
        bool initialized;

        void fontResource();
    };

}
