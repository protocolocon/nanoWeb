/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "ml_parser.h"
#include <cctype>
#include <stdlib.h>

using namespace std;

namespace webui {

    MLParser::~MLParser() {
        if (ownOrig)
            free(const_cast<char*>(mlOrig));
    }

    bool MLParser::parse(const char* ml, int n, bool value) {
        mlOrig = ml;
        mlEnd = ml + n;
        DIAG(line = 1);
        DIAG(errorFlag =) ownOrig = false;
        clear();
        if (value) {
            if (skipSpace(ml))
                return DIAG(error(ml, "no value found") &&) false;
            if (parseValue(ml))
                return DIAG(error(ml, "parse value") &&) false;
            if (!skipSpace(ml))
                return DIAG(error(ml, "trailing content in value parser") &&) false;
        } else {
            if (!parseLevel(ml))
                return DIAG(error(ml, "trailing content in object parser") &&) false;
        }
        return true DIAG(&& !errorFlag);
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
            if (skipId(ml) || skipSpace(ml)) return DIAG(error(ml, "id with no object") &&) false;
            if (*ml != '{') return DIAG(error(ml, "expected object defitition") &&) false;
            newEntry(ml, prev); ++ml; // object
            parseObject(ml);
            return skipSpace(ml);
        } else
            return DIAG(error(ml, "expecting object identifier") &&) false;
    }

    bool MLParser::parseObject(const char*& ml) {
        /* expecting:
             key: value,
             <object id> {
             }
        */
        int prev(-1);
        while (ml < mlEnd) {
            if (skipSpace(ml)) return DIAG(error(ml, "unsinished object") &&) false;
            if (*ml == '}') { ++ml; break; } // object definition finished
            if (isalpha(*ml)) {
                prev = newEntry(ml, prev); // id
                if (skipId(ml) || skipSpace(ml)) return DIAG(error(ml, "key w/o value or object w/o definition") &&) false;
                if (*ml == ':') {
                    ++ml;
                    if (skipSpace(ml)) return DIAG(error(ml, "EOF expecting value") &&) false;
                    prev = newEntry(ml, prev);
                    if (parseValue(ml)) return DIAG(error(ml, "expeting value") &&) false;
                    if (skipSpace(ml)) return DIAG(error(ml, "EOF expecting '}' or ','") &&) false;
                    if (*ml == ',') ++ml;
                }
                else if (*ml == '{') {
                    prev = newEntry(ml, prev);
                    ++ml;
                    if (parseObject(ml)) DIAG(error(ml, "EOF parsing object"));
                }
                else return DIAG(error(ml, "expecting ':' or '{' after id") &&) false;
            } else
                return DIAG(error(ml, "expecting <id> or '}'") &&) false;
        }
        return false;
    }

    bool MLParser::parseList(const char*& ml, char expectedEndChar) {
        /* expecting:
           <value>[, <value>[, ...]]
        */
        if (skipSpace(ml)) return DIAG(error(ml, "unsinished list") &&) false;
        if (*ml == expectedEndChar) { ++ml; return false; }
        int prev(-1);
        while (ml < mlEnd) {
            prev = newEntry(ml, prev);
            if (parseValue(ml) DIAG(|| errorFlag)) return DIAG(error(ml, "EOF expecting value") &&) false;
            if (skipSpace(ml)) return DIAG(error(ml, "unfinished list") &&) false;
            if (*ml == expectedEndChar) { ++ml; return false; }
            if (*ml != ',') return DIAG(error(ml, "expecting ',' to separate values") &&) false;
            ++ml;
            if (skipSpace(ml)) return DIAG(error(ml, "unfinished list") &&) false;
        }
        return true;
    }

    bool MLParser::parseValue(const char*& ml) {
        /* expecting:
           [ ... ]
           { ... }
           " ... "
           <id>
           @
        */
        if (*ml == '[') { ++ml; return parseList(ml, ']'); }
        else if (*ml == '{') { ++ml; return parseObject(ml); }
        else if (*ml == '"') return skipString(ml);
        else if (isalnum(*ml) || *ml == '-' || *ml == '@') {
            if (skipId(ml)) return DIAG(error(ml, "EOF parsing id") &&) false;
            if (skipSpace(ml)) return DIAG(error(ml, "EOF skipping space") &&) false;
            if (*ml == '(') { ++ml; return parseList(ml, ')'); } // function parameters
            return false;
        }
        else return DIAG(error(ml, "expecting value") &&) false;
    }

    int MLParser::newEntry(const char* pos, int prev) {
        if (prev >= 0) entries[prev].next = entries.size();
        entries.push_back(Entry(pos DIAG(, line)));
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
            if (*ml == '\n') { DIAG(line++); ++ml; }
            else if (isspace(*ml)) ++ml;
            else if (*ml == '/' && ml[1] == '/') skipLine(ml); // comment
            else return false;
        }
        return true;
    }

    bool MLParser::skipLine(const char*& ml) const {
        while (ml < mlEnd) {
            if (*ml == '\n') { DIAG(line++); ++ml; return false; }
            ++ml;
        }
        return true;
    }

    bool MLParser::skipSimpleValue(const char*& ml) const {
        if (*ml == '"') return skipString(ml);
        return skipId(ml);
    }

    DIAG(
        bool MLParser::error(const char* ml, const char* msg, int line_) const {
            errorFlag = true;
            if (!line_) line_ = line;
            LOG("error parsing ML: %s at line %d (document position %ld)", msg, line_, ml - mlOrig);
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
        });

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

    void MLParser::copyTo(MLParser& dst, int iEntry, int jEntry) const {
        int size( (jEntry < int(entries.size()) ? entries[jEntry].pos : mlEnd) - entries[iEntry].pos );
        dst.mlOrig = strndup(entries[iEntry].pos, size);
        dst.mlEnd = dst.mlOrig + size;
        DIAG(dst.errorFlag = false);
        dst.ownOrig = true;
        dst.entries.assign(entries.begin() + iEntry, entries.begin() + jEntry);
        // fix next indices and text positions in dst parser
        for (auto& entry: dst.entries) {
            entry.pos += dst.mlOrig - entries[iEntry].pos;
            entry.next = entry.next ? entry.next - iEntry : 0;
        }
    }

    void MLParser::swap(MLParser& o) {
        std::swap(mlOrig, o.mlOrig);
        std::swap(mlEnd, o.mlEnd);
        DIAG(std::swap(line, o.line));
        DIAG(std::swap(errorFlag, o.errorFlag));
        std::swap(ownOrig, o.ownOrig);
        entries.swap(o.entries);
    }

    DIAG(
        void MLParser::dump() const {
            LOG("parser entries: %zu", entries.size());
            char buffer[20];
            buffer[sizeof(buffer) - 1] = 0;
            for (const auto& entry: entries) {
                for (int i = 0; i < int(sizeof(buffer)) - 1; i++)
                    buffer[i] = (entry.pos + i < mlEnd && entry.pos[i] >= 32) ? entry.pos[i] : '.';
                LOG("%4d |%s| %4d", int(&entry - entries.data()), buffer, entry.next);
            }
        });

    DIAG(
        void MLParser::dumpTree() const {
            LOG("parser entries tree");
            dumpTreeRecur(0, entries.size(), 0);
        });

    DIAG(
        void MLParser::dumpTreeRecur(int iEntry, int fEntry, int level) const {
            while (iEntry < fEntry) {
                const auto& entry(entries[iEntry]);
                int next(entry.next ? entry.next : fEntry);
                if (next > iEntry + 1) {// children
                    if (isalnum(*entry.pos)) {
                        auto strSize(entry.asStrSize(*this, true));
                        LOG("%*s%d %.*s", level*2, "", iEntry, strSize.second, strSize.first);
                    } else
                        LOG("%*s%d %c", level*2, "", iEntry, *entry.pos);
                    dumpTreeRecur(iEntry + 1, next, level + 1);
                } else {
                    auto strSize(entry.asStrSize(*this, true));
                    LOG("%*s%d %.*s", level*2, "", iEntry, strSize.second, strSize.first);
                }
                iEntry = next;
            }
        });

    // Entry
    pair<const char*, int> MLParser::Entry::asStrSize(const MLParser& parser, bool quotes) const {
        auto ml(pos);
        if (!parser.skipSimpleValue(ml)) {
            if (!quotes && *pos == '"') return make_pair(pos + 1, ml - pos - 2);
            return make_pair(pos, ml - pos);
        }
        return make_pair(nullptr, 0);
    }

    Identifier MLParser::Entry::asId(const MLParser& parser, const StringManager& strMng) const {
        auto ml(pos);
        if (!parser.skipSimpleValue(ml)) return Identifier(strMng.search(pos, ml - pos).getId());
        return Identifier::InvalidId;
    }

    StringId MLParser::Entry::asStrId(const MLParser& parser, StringManager& strMng, bool quotes) const {
        auto ml(pos);
        if (!parser.skipSimpleValue(ml)) {
            if (!quotes && *pos == '"') return strMng.add(pos + 1, ml - pos - 2);
            return strMng.add(pos, ml - pos);
        }
        return StringId();
    }

}
