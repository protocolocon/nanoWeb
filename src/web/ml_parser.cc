/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "ml_parser.h"
#include <cctype>

using namespace std;

namespace webui {

    bool MLParser::parse(const char* ml, int n) {
        mlOrig = ml;
        mlEnd = ml + n;
        line = 1;
        errorFlag = false;
        clear();
        if (!parseLevel(ml))
            return error(ml, "trailing content");
        return true;
    }

    bool MLParser::parseLevel(const char*& ml) {
        /* expecting:
             <object id> {
               ...
             }
         */
        if (skipSpace(ml)) return true;
        if (isalpha(*ml)) {
            // named object
            int prev(newEntry(ml)); // id
            if (skipId(ml) || skipSpace(ml)) return error(ml, "id with no object");
            if (*ml != '{') return error(ml, "expected object defitition");
            newEntry(ml, prev); ++ml; // object
            parseObject(ml);
            return skipSpace(ml);
        } else
            return error(ml, "expecting object identifier");
    }

    bool MLParser::parseObject(const char*& ml) {
        /* expecting:
             key: value,
             <object id> {
             }
        */
        int prev(-1);
        while (ml < mlEnd) {
            if (skipSpace(ml)) return error(ml, "unsinished object");
            if (*ml == '}') { ++ml; break; } // object definition finished
            if (isalpha(*ml)) {
                prev = newEntry(ml, prev); // id
                if (skipId(ml) || skipSpace(ml)) return error(ml, "key w/o value or object w/o definition");
                if (*ml == ':') {
                    ++ml;
                    if (skipSpace(ml)) return error(ml, "EOF expecting value");
                    prev = newEntry(ml, prev);
                    if (parseValue(ml)) return error(ml, "expeting value");
                    if (skipSpace(ml)) return error(ml, "EOF expecting '}' or ','");
                    if (*ml == ',') ++ml;
                }
                else if (*ml == '{') {
                    prev = newEntry(ml, prev);
                    ++ml;
                    if (parseObject(ml)) error(ml, "EOF parsing object");
                }
                else return error(ml, "expecting ':' or '{' after id");
            } else
                return error(ml, "expecting <id> or '}'");
        }
        return false;
    }

    bool MLParser::parseList(const char*& ml, char expectedEndChar) {
        /* expecting:
           <value>[, <value>[, ...]]
        */
        if (skipSpace(ml)) return error(ml, "unsinished list");
        if (*ml == expectedEndChar) { ++ml; return false; }
        int prev(-1);
        while (ml < mlEnd) {
            prev = newEntry(ml, prev);
            if (parseValue(ml) || errorFlag) return error(ml, "expecting value");
            if (skipSpace(ml)) return error(ml, "unfinished list");
            if (*ml == expectedEndChar) { ++ml; break; }
            if (*ml != ',') return error(ml, "expecting ',' to separate values");
            ++ml;
            if (skipSpace(ml)) return error(ml, "unfinished list");
        }
        return false;
    }

    bool MLParser::parseValue(const char*& ml) {
        /* expecting:
           [ ... ]
           { ... }
           " ... "
           <id>
        */
        if (*ml == '[') { ++ml; return parseList(ml, ']'); }
        else if (*ml == '{') { ++ml; return parseObject(ml); }
        else if (*ml == '"') return skipString(ml);
        else if (isalnum(*ml)) {
            if (skipId(ml)) return error(ml, "EOF parsing id");
            if (skipSpace(ml)) return error(ml, "EOF skipping space");
            if (*ml == '(') { ++ml; return parseList(ml, ')'); } // function parameters
            return false;
        }
        else return error(ml, "expecting value");
    }

    int MLParser::newEntry(const char* pos, int prev) {
        if (prev >= 0) entries[prev].next = entries.size();
        entries.push_back(pos);
        return entries.size() - 1;
    }

    bool MLParser::skipId(const char*& ml) const {
        while (ml < mlEnd) {
            if (!*ml || *ml == ':' || *ml == '{' || *ml == '(' || *ml == ',' || *ml == ']' || *ml == ')' || isspace(*ml)) return false;
            ++ml;
        }
        return true;
    }

    bool MLParser::skipString(const char*& ml) const {
        ++ml; // '"'
        while (ml < mlEnd) {
            if (*ml == '"') { ++ml; return false; }
            if (*ml == '\\' && *ml == '"') ++ml;
            ++ml;
        }
        return true;
    }

    bool MLParser::skipSpace(const char*& ml) const {
        while (ml < mlEnd) {
            if (*ml == '\n') { line++; ++ml; }
            else if (isspace(*ml)) ++ml;
            else if (*ml == '/' && ml[1] == '/') skipLine(ml); // comment
            else return false;
        }
        return true;
    }

    bool MLParser::skipLine(const char*& ml) const {
        while (ml < mlEnd) {
            if (*ml == '\n') { line++; ++ml; return false; }
            ++ml;
        }
        return true;
    }

    bool MLParser::skipValue(const char*& ml) const {
        if (*ml == '"') return skipString(ml);
        else if (isalnum(*ml)) return skipId(ml);
        return false;
    }

    bool MLParser::error(const char* ml, const char* msg) {
        errorFlag = true;
        LOG("error parsing ML: %s at line %d (document position %ld)", msg, line, ml - mlOrig);
        char buffer[64 + 1];
        for (int i = -32; i < 32; i++) {
            if (ml + i >= mlOrig && ml +i < mlEnd && ml[i] >= 32)
                buffer[i + 32] = ml[i];
            else
                buffer[i + 32] = '.';
        }
        buffer[64] = 0;
        LOG("around: |%s|", buffer);
        LOG("        |%32s^%31s|", "", "");
        return false;
    }

    int MLParser::getLevelEnd(int iEntry) const {
        int i(iEntry);
        while (--i >= 0) {
            const auto& entry(entries[i]);
            if (entry.next > iEntry) return entry.next;
        }
        return int(entries.size());
    }

    int MLParser::getTemporalEntry(const char* text) {
        entries.reserve(entries.size() + 1);
        auto& entry(entries[entries.size()]);
        entry.pos = text;
        entry.next = 0;
        mlEnd = text + strlen(text) + 1;
        return entries.size();
    }

    void MLParser::dump() const {
        char buffer[20];
        buffer[sizeof(buffer) - 1] = 0;
        for (const auto& entry: entries) {
            for (int i = 0; i < int(sizeof(buffer)) - 1; i++)
                buffer[i] = (entry.pos + i < mlEnd && entry.pos[i] >= 32) ? entry.pos[i] : '.';
            LOG("%4ld |%s| %4d", &entry - entries.data(), buffer, entry.next);
        }
    }

    void MLParser::dumpTree() const {
        dumpTreeRecur(0, entries.size(), 0);
    }

    void MLParser::dumpTreeRecur(int iEntry, int fEntry, int level) const {
        while (iEntry < fEntry) {
            const auto& entry(entries[iEntry]);
            int next(entry.next ? entry.next : fEntry);
            if (next > iEntry + 1) {// children
                if (isalnum(*entry.pos)) {
                    auto strSize(entry.asStrSize(*this));
                    LOG("%*s%d %.*s", level*2, "", iEntry, strSize.second, strSize.first);
                }
                dumpTreeRecur(iEntry + 1, next, level + 1);
            } else {
                auto strSize(entry.asStrSize(*this));
                LOG("%*s%d %.*s", level*2, "", iEntry, strSize.second, strSize.first);
            }
            iEntry = next;
        }
    }

    // Entry
    pair<const char*, int> MLParser::Entry::asStrSize(const MLParser& parser) const {
        auto ml(pos);
        if (!parser.skipValue(ml)) {
            if (*pos == '"') return make_pair(pos + 1, ml - pos - 2);
            return make_pair(pos, ml - pos);
        }
        return make_pair(nullptr, 0);
    }

    Identifier MLParser::Entry::asId(const MLParser& parser, const StringManager& strMng) const {
        auto ml(pos);
        if (!parser.skipValue(ml)) return Identifier(strMng.search(pos, ml - pos).getId());
        return Identifier::InvalidId;
    }

    StringId MLParser::Entry::asStrId(const MLParser& parser, StringManager& strMng) const {
        auto ml(pos);
        if (!parser.skipValue(ml)) {
            if (*pos == '"') return strMng.add(pos + 1, ml - pos - 2);
            return strMng.add(pos, ml - pos);
        }
        return StringId();
    }

}
