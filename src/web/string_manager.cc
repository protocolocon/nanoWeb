/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "string_manager.h"
#include "compatibility.h"
#include <cstring>

namespace webui {

    StringId StringManager::add(const char *str, int nStr) {
        if (!str) return -1;
        if (nStr < 0) nStr = strlen(str);
        int iIndex(searchNearestIndex(str, nStr));
        if (iIndex < int(index.size()) &&
            !strncmp(&doc[index[iIndex]], str, nStr) &&
            !doc[index[iIndex] + nStr])
            return index[iIndex];

        // add string
        auto i(doc.size());
        doc.resize(doc.size() + nStr + 1);
        memcpy(&doc[i], str, nStr);
        doc[i + nStr] = 0;

        // add index
        index.insert(index.begin() + iIndex, i);
        return i;
    }

    int StringManager::searchNearestIndex(const char *str) const {
        int ini(0), end(int(index.size()));
        while (end > ini) {
            int mid((ini + end) >> 1);
            if (strcmp(&doc[index[mid]], str) >= 0) end = mid; else ini = mid + 1;
        }
        return ini;
    }

    int StringManager::searchNearestIndex(const char* str, int nStr) const {
        int ini(0), end(int(index.size()));
        while (end > ini) {
            int mid((ini + end) >> 1);
            if (strncmp(&doc[index[mid]], str, nStr) >= 0) end = mid; else ini = mid + 1;
        }
        return ini;
    }

    StringId StringManager::search(const char *str) const {
        int ini(searchNearestIndex(str));
        return (ini < int(index.size()) && !strcmp(&doc[index[ini]], str)) ? index[ini] : -1;
    }

    StringId StringManager::search(const char* str, int nStr) const {
        int ini(searchNearestIndex(str, nStr));
        return (ini < int(index.size()) &&
                !strncmp(&doc[index[ini]], str, nStr) &&
                !doc[index[ini] + nStr]) ? index[ini] : -1;
    }

    StringId StringManager::searchPrefix(const char *str) const {
        int ini(searchNearestIndex(str));
        return (ini < int(index.size()) && !strncmp(&doc[index[ini]], str, strlen(str))) ? index[ini] : -1;
    }

    void StringManager::dump() const {
        LOG("size of doc: %8zu", doc.size());
        LOG("# strings:   %8zu", index.size());
        for (auto i: index)
            LOG("\t%8d {%s}", i, &doc[index[i]]);
    }

}
