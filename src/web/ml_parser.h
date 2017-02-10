/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include <vector>

namespace webui {

    class MLParser {
    public:
        MLParser(): ownOrig(false), mlOrig(nullptr), mlEnd(nullptr) { }
        ~MLParser();

        bool parse(const char* ml, int n);

        enum class EntryType {
            Unknown,
            Id,
            Object,
            Block,
            Function,
            List,
            Number,
            Color,
            String,
            Operator,
            Wildcar,
            Attribute,
        };

        DIAG(static const char* toString(EntryType t));

        struct Entry {
            Entry(EntryType type, const char* pos = nullptr DIAG(, int line = 0)): pos(pos), next(0), type_(int(type)) DIAG(, line(line)) { }
            inline EntryType type() const { return EntryType(type_); }
            inline void setType(EntryType type) { type_ = int(type); }

            const char* pos;
            int next:28;
            uint32_t type_:4;
            DIAG(int line);
        };

        inline bool empty() const { return entries.empty(); }
        inline void clear() { entries.clear(); }
        inline int size() const { return entries.size(); }
        inline Entry& operator[](int i) { return entries[i]; }
        inline const Entry& operator[](int i) const { return entries[i]; }
        int size(int iEntry) const;
        void swap(MLParser& o);
        inline void swapEnd(MLParser& other) { std::swap(mlEnd, other.mlEnd); }
        int getTemporalEntry(const char* text);
        void copyTo(MLParser& dst, int iEntry, int jEntry) const;

        // get elements of ML
        Identifier asId(int iEntry) const;
        Identifier asIdAdd(int iEntry) const;

        // returns false
        DIAG(bool error(const char* ml, const char* msg, int line = 0) const);
        DIAG(void dumpTree() const);

    private:
        bool ownOrig;                // wether MLParser owns mlOrig memory (needs to free on dtor) or not
        const char* mlOrig;
        const char* mlEnd;
        DIAG(mutable int line);
        std::vector<Entry> entries;

        void finish();

        inline char get(const char* ml) const { return ml < mlEnd ? *ml : 0; }
        int newEntry(EntryType type, const char* pos, int prev);

        char skipSpace(const char*&ml) const;
        void skipLine(const char*&ml) const;
        void skipId(const char*&ml) const;
        bool skipNumber(const char*&ml) const;
        bool skipColor(const char*&ml) const;
        bool skipString(const char*&ml) const;
        int parseId(const char*&ml, int prev);
        int parseNumber(const char*&ml, int prev);
        int parseColor(const char*&ml, int prev);
        int parseString(const char*&ml, int prev);
        int parseExpression(const char*&ml, int prev, bool op = false);
        int parseObject(const char*&ml, int prev, char endChar);
        bool parseList(const char*&ml, char endChar);

        // returns the size of the operator or 0 if no operator found
        static int isOperator(const char* c);

        void fixLevelEndings(int iEntry, int jEntry);

        DIAG(void dumpTreeRecur(int iEntry, int fEntry, int level) const);
    };

}
