/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "compatibility.h"
#include <vector>
#include <string>

namespace webui {

    class MLParser {
    public:
        void parse(const char* ml, int n);

        void dump() const;
        void dumpTree() const;

        struct Entry {
            Entry(const char* pos = nullptr): pos(pos), next(0) { }
            std::string getSimpleValue(const MLParser& parser) const; // string, id or number
            const char* pos;
            int next;
        };

    private:
        const char* mlOrig;
        const char* mlEnd;
        mutable int line;
        bool errorFlag;
        std::vector<Entry> entries;

        int newEntry(const char* pos, int prev = -1);

        // return true on EOF
        bool parseLevel(const char*& ml);
        bool parseObject(const char*& ml);
        bool parseValue(const char*& ml);
        bool parseList(const char*& ml);

        // return true on EOF
        bool skipSpace(const char*& ml) const;
        bool skipLine(const char*& ml) const;
        bool skipId(const char*& ml) const;
        bool skipString(const char*& ml) const;

        // returns false
        bool error(const char* ml, const char* msg);

        // debug
        void dumpTreeRecur(int iEntry, int fEntry, int level) const;
    };

}
