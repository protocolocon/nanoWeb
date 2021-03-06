/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <thread>
#include <cstring>
#include <cassert>
#include <sys/time.h>
#include <curl/curl.h>

using namespace std;
using namespace webui;

namespace {

    char curlErrorBuffer[CURL_ERROR_SIZE];
    const char* curlServerAddr("127.0.0.1:9999");

    // cursors
    GLFWcursor* cursors[int(Cursor::Last)];

    bool mainLoopRunning(true);
}

namespace webui {

    void setCommandLine(int argc, char** argv) {
        // initialize curl
        curl_global_init(CURL_GLOBAL_DEFAULT);

        // get server address
        if (argc >= 2) {
            curlServerAddr = argv[1];
            LOG("using specified server: %s", curlServerAddr);
        } else {
            LOG("no server address specified, using: %s", curlServerAddr);
            LOG("use: %s [<domain | IP>[:<port>]]", argv[0]);
        }
    }

    void setMainLoop(void (*loop)(void)) {
        // initialize cursors
        cursors[1] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
        cursors[2] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

        // synchronous main loop
        while (mainLoopRunning) {
            loop();
            this_thread::sleep_for(chrono::milliseconds(10));
        };
    }

    void cancelMainLoop() {
        mainLoopRunning = false;
    }

    int defaultWidth() {
        return 1280;
    }

    int defaultHeight() {
        return 1024;
    }

    int getTimeNowMs() {
        struct timeval now;
        gettimeofday(&now, nullptr);
        return now.tv_sec * 1000 + now.tv_usec / 1000;
    }


    // cursors
    void setCursorInner(Cursor c) {
        assert(size_t(c) < sizeof(cursors) / sizeof(cursors[0]));
        glfwSetCursor(Context::render.getWin(), cursors[int(c)]);
    }


    // class RequestXHR
    RequestXHR::RequestXHR(StringId id, StringId req, const char* data_, int nData):
        id(id), req(req), data(nullptr), nData(nData) {
        if (nData && data_) {
            data = (char*)malloc(nData);
            memcpy(data, data_, nData);
        }
    }

    RequestXHR::~RequestXHR() {
        free(data);
    }

    void RequestXHR::query() {
        char buffer[1024];
        const char* req(buildQuery(buffer, sizeof(buffer)));
        // perform the request synchronously
        CURL *conn(curl_easy_init());
        if (!conn) {
            LOG("libcurl: error setting connection");
            return;
        }
        CURLcode code;
        if (CURLE_OK != (code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, curlErrorBuffer))) {
            LOG("libcurl: error setting error buffer %d", code);
            return;
        }
        // compose URL
        string url("http://"s + curlServerAddr + '/' + req);
        LOG("{%s}", url.c_str());

        if (CURLE_OK != (code = curl_easy_setopt(conn, CURLOPT_URL, url.c_str()))) {
            LOG("libcurl: error setting URL %d: %s", code, curlErrorBuffer);
            return;
        }
        if (CURLE_OK != (code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L))) {
            LOG("libcurl: error setting redirections %d: %s", code, curlErrorBuffer);
            return;
        }
        if (CURLE_OK != (code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, onAddDataStatic))) {
            LOG("libcurl: error setting writer %d: %s", code, curlErrorBuffer);
            return;
        }
        if (CURLE_OK != (code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, this))) {
            LOG("libcurl: error setting writer data %d: %s", code, curlErrorBuffer);
            return;
        }

        long status;
        code = curl_easy_perform(conn);
        curl_easy_getinfo(conn, CURLINFO_RESPONSE_CODE, &status);
        curl_easy_cleanup(conn);

        if (CURLE_OK != code || status != 200)
            onError();
        else
            onLoad(data, nData);
    }

    size_t RequestXHR::onAddData(char* newData, size_t size, size_t nmemb) {
        size_t newSize(size * nmemb);
        data = (char*)realloc(data, nData + newSize + 1);
        memcpy(data + nData, newData, newSize);
        nData += newSize;
        return newSize;
    }

}
