/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "json.h"
#include <algorithm>

using namespace std;

namespace webui {

    Application::Application(): init(false) {
    }

    void Application::refresh() {
        if (!init) initialize();
    }

    void Application::initialize() {
        if (xhr.getStatus() == RequestXHR::Empty) xhr.query("application.json");
        else if (xhr.getStatus() == RequestXHR::Ready) {
            init = true;
            xhr.makeCString();

            JSON json[16384];
            int nJson = jsonparse(xhr.getData(), json, sizeof(json) / sizeof(JSON));
            if (nJson) parseDescription(json, json + nJson); else LOG("failed parsing json: %s", xhr.getData());
            xhr.clear();
        }
    }

    void Application::parseDescription(JSON* json, JSON* end) {
        char orig(0);
        do {
            swap(*const_cast<char*>(json->end), orig);
            LOG("Type: %c  Size: %d  Value: %s", *json->src, end - json, json->src);
            swap(*const_cast<char*>(json->end), orig);
            if (json + 1 < end && (json + 1)->parent == json) // children
                parseDescription(json + 1, json->next ? json->next : end);
            else if (json + 1 < end && (!json->next || json->next > json + 1)) // key value
                parseDescription(json + 1, json->next ? json->next : end);
        } while ((json = json->next) && json < end);
    }

}
