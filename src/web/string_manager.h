/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <vector>
#include "reserved_words.h"

namespace webui {

    class StringId {
    public:
        StringId(): id(-1) { }
        StringId(int id): id(id) { }
        StringId(Identifier id): StringId(int(id)) { }
        inline int getId() const { return id; }
        inline bool valid() const { return id >= 0; }
        inline std::size_t operator()(StringId id) const { return id.getId(); }
        inline bool operator==(StringId rhs) const { return id == rhs.id; }
        inline bool operator>=(StringId rhs) const { return id >= rhs.id; }
        inline bool operator<=(StringId rhs) const { return id <= rhs.id; }

    private:
        int id;
    };

    class StringManager {
        // string storage
        std::vector<char> doc;

        // index to strings in doc (so that strings are sorted)
        std::vector<int> index;

        int searchNearestIndex(const char* str) const;
        int searchNearestIndex(const char* str, int nStr) const;

    public:
        // get closest even if there's no match
        StringId searchNearest(const char* str) const;
        StringId searchNearest(const char* str, int nStr) const;

        // returns index of string or -1 if string not in pool
        // O(log(N)
        // in the case of prefix string, searched string must start by prefix
        StringId search(const char* str) const;
        StringId search(const char* str, int nStr) const;
        StringId searchPrefix(const char* prefix) const;

        // returns index of newly created string or the index of existing replica
        StringId add(const char* str, int nStr = -1);

        // returns the string by doc index or NULL if index==-1
        inline const char* get(StringId id) const { return id.getId() < 0 ? nullptr : &doc[id.getId()]; }
        inline const char* get(Identifier id) const { return get(StringId(int(id))); }

        // debug
        void dump() const;
    };

}
