/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include "string_manager.h"
#include "reserved_words.h"
#include <vector>
#include <string>

namespace webui {

    class MLParser {
    public:
        MLParser(): ownOrig(false) { }
        ~MLParser();

        // returns false on error
        bool parse(const char* ml, int n, bool value = false);

        struct Entry {
            Entry(const char* pos = nullptr DIAG(, int line = 0)): pos(pos), next(0) DIAG(, line(line)) { }
            std::pair<const char*, int> asStrSize(const MLParser& parser, bool quotes) const;   // string, id or number
            Identifier asId(const MLParser& parser, const StringManager& strMng) const;         // string, id or number
            StringId asStrId(const MLParser& parser, StringManager& strMng, bool quotes) const; // string, id or number
            const char* pos;
            int next;
            DIAG(int line);
        };

        inline void clear() { entries.clear(); }
        inline int size() const { return entries.size(); }
        inline Entry& operator[](int i) { return entries[i]; }
        inline const Entry& operator[](int i) const { return entries[i]; }
        int getLevelEnd(int iEntry) const;

        int getTemporalEntry(const char* text);

        void copyTo(MLParser& dst, int iEntry, int jEntry) const;
        void swap(MLParser& other);
        inline void swapEnd(MLParser& other) { std::swap(mlEnd, other.mlEnd); }

        // returns false
        DIAG(bool error(const char* ml, const char* msg, int line = 0) const);

        DIAG(void dump() const);
        DIAG(void dumpTree() const);

    private:
        const char* mlOrig;
        const char* mlEnd;
        DIAG(mutable int line);
        DIAG(mutable bool errorFlag);
        bool ownOrig;
        std::vector<Entry> entries;

        int newEntry(const char* pos, int prev = -1);

        // return true on EOF
        bool parseLevel(const char*& ml);
        bool parseObject(const char*& ml);
        bool parseValue(const char*& ml);
        bool parseList(const char*& ml, char expectedEndChar);

        // return true on EOF
        bool skipSpace(const char*& ml) const;
        bool skipLine(const char*& ml) const;
        bool skipId(const char*& ml) const;
        bool skipString(const char*& ml) const;
        bool skipSimpleValue(const char*& ml) const;

        // debug
        DIAG(void dumpTreeRecur(int iEntry, int fEntry, int level) const);
    };

}
