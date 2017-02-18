/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "ml_parser.h"
#include "context.h"
#include <cctype>
#include <cassert>
#include <cstdlib>

#ifdef DISABLE_DIAGNOSTICS
#  define ERROR_FALSE(ml, msg) false
#  define ERROR_INT(ml, msg)   -1
#else
#  define ERROR_FALSE(ml, msg) error(ml, msg, line)
#  define ERROR_INT(ml, msg)   error(ml, msg, line), -1
#endif

using namespace std;
using namespace webui;

namespace webui {

    DIAG(const char* MLParser::toString(MLParser::EntryType type) {
            static const char* typeNames[] = {
                "unknown",
                "id",
                "object",
                "block",
                "function",
                "list",
                "number",
                "color",
                "string",
                "operator",
                "wildcar",
                "attribute",
            };
            return typeNames[int(type)];
        });

    MLParser::~MLParser() {
        finish();
    }

    void MLParser::finish() {
        if (ownOrig)
            free(const_cast<char*>(mlOrig));
    }

    bool MLParser::parse(const char* ml, int n) {
        finish();
        mlOrig = ml;
        mlEnd = ml + n;
        DIAG(line = 1);
        entries.clear();
        if (parseExpression(ml, -1) >= 0) {
            char c = skipSpace(ml);
            if (c) return ERROR_FALSE(ml, "expecting EOF");
            fixLevelEndings(0, entries.size());
            // reserve memory for last entry
            newEntry(EntryType::Unknown, nullptr, -1);
            entries.pop_back();
            return true;
        }
        return false;
    }

    void MLParser::fixLevelEndings(int iEntry, int jEntry) {
        while (iEntry < jEntry) {
            auto& entry(entries[iEntry]);
            if (!entry.next) entry.next = jEntry;
            if (entry.next != iEntry + 1)
                fixLevelEndings(iEntry + 1, entry.next);
            iEntry = entry.next;
        }
    }

    int MLParser::parseExpression(const char*&ml, int prev) {
        int iExpression(entries.size());
        if ((prev = parseExpressionRecur(ml, prev)) < 0) return -1;
        entries[iExpression].next = 0; // expression has one entry point, everything else is nested
        return iExpression;
    }

    int MLParser::parseExpressionRecur(const char*&ml, int prev, const char* op) {
        // expecting: id, function, formula, list, object, color, number or string
        int prevOrig(prev);
        auto c(skipSpace(ml));
        // id
        if (isalpha(c)) {
            prev = parseId(ml, prev);
            c = skipSpace(ml);
            // object
            if (c == '{') {
                if (op) return ERROR_INT(ml, "object cannot be used in operation");
                entries[prev].setType(EntryType::Object);
                ++ml;
                if (parseObject(ml, -1, '}') < 0) return -1;
                return prev;
            }
            while (true) {
                // function call (continue for formula)
                if (c == '(') {
                    entries[prev].setType(EntryType::Function);
                    ++ml;
                    if (!parseList(ml, ')')) return -1;
                    break;
                } else if (c == '.') {
                    // attribute
                    ++ml;
                    skipSpace(ml);
                    prev = parseId(ml, prev);
                    entries[prev].setType(EntryType::Attribute);
                    c = skipSpace(ml);
                } else break;
            }
        } else if (c == '[') { // list
            if (op) return ERROR_INT(ml, "list cannot be used in operation");
            prev = newEntry(EntryType::List, ml++, prev);
            return parseList(ml, ']') ? prev : -1;
        } else if (c =='#') { // color (continue for formula)
            if ((prev = parseColor(ml, prev)) < 0) return -1;
        } else if (c == '"' || c == '\'') { // string
            if (op && *op != '=') return ERROR_INT(ml, "string cannot be used in operation");
            return parseString(ml, prev);
        } else if (c == '(') { // operation parenthesis
            ++ml;
            if ((prev = parseExpressionRecur(ml, prev)) < 0) return -1;
            c = skipSpace(ml);
            if (c != ')') return ERROR_INT(ml, "expecting ')' in operation");
            ++ml;
            --prevOrig;
        } else if ((!op && c == '-') || c == '.' || isdigit(c)) { // number
            if ((prev = parseNumber(ml, prev)) < 0) return -1;
        } else if (c == '@') { // wildcar
            prev = newEntry(EntryType::Wildcar, ml++, prev);
        }
        // operators, a way of continuing with expression
        skipSpace(ml);
        int opSize;
        if ((opSize = isOperator(ml))) {
            if (prev == -1 || entries.empty() || entries.back().type() == EntryType::Operator) return ERROR_INT(ml, "invalid operator position");
            prev = newEntry(EntryType::Operator, ml, prev);
            ml += opSize;
            if (parseExpressionRecur(ml, -1, ml - opSize) < 0) return -1;
        }
        DIAG(if (!entries.empty() && entries.back().type() == EntryType::Operator) {
                error(ml, "expression ending in operator", line);
                return -1;
            });
        if (prev == prevOrig) return ERROR_INT(ml, "expecting expression");
        return prev;
    }

    int MLParser::parseObject(const char*&ml, int prev, char endChar) {
        // expecting:
        //    key: expression
        //    object { }
        //    [ ]  <- block
        int prevInner(-1);
        while (true) {
            auto c(skipSpace(ml));
            if (c == endChar) { // end
                ++ml;
                return 0;
            } else if (c == '[') { // block
                prevInner = newEntry(EntryType::Block, ml++, prevInner);
                if (parseObject(ml, -1, ']') < 0) return -1;
            } else if (isalpha(c)) { // key: expression or object { }
                if ((prevInner = parseId(ml, prevInner)) < 0) return -1;
                c = skipSpace(ml);
                if (c == '{') { // object { }
                    entries[prevInner].setType(EntryType::Object);
                    ++ml;
                    if (parseObject(ml, -1, '}') < 0) return -1;
                } else if (c == ':') { // key: expression
                    ++ml;
                    if ((prevInner = parseExpression(ml, prevInner)) < 0) return -1;
                } else {
                    return ERROR_INT(ml, "expecting object '{' or key value expression ':'");
                }
            } else {
                return ERROR_INT(ml, "expecting object end '}|]' or block '[' or id");
            }
        }
    }

    bool MLParser::parseList(const char*&ml, char endChar) {
        // expecting: [expression[, expression[, ...]]] endchar
        auto c(skipSpace(ml));
        if (c == endChar) { ++ml; return true; }
        int prev(-1);
        while (true) {
            if ((prev = parseExpression(ml, prev)) < 0) return false;
            c = skipSpace(ml);
            if (c == ',') ++ml;
            else if (c == endChar) { ++ml; return true; }
            else return ERROR_FALSE(ml, "expecting ',' or end of list");
        }
    }

    int MLParser::parseNumber(const char*&ml, int prev) {
        prev = newEntry(EntryType::Number, ml, prev);
        if (!skipNumber(ml)) return -1;
        return prev;
    }

    bool MLParser::skipNumber(const char*&ml) const {
        if (get(ml) == '-') ++ml;
        bool dot(false);
        while (true) {
            auto c(get(ml));
            if (c == '.') {
                if (dot) return ERROR_FALSE(ml, "several dots in number");
                dot = true;
            } else if (isdigit(c)) {
            } else if (isalpha(c)) return ERROR_FALSE(ml, "invalid character for number");
            else return true;
            ++ml;
        }
    }

    int MLParser::parseColor(const char*&ml, int prev) {
        prev = newEntry(EntryType::Color, ml, prev);
        if (!skipColor(ml)) return -1;
        return prev;
    }

    bool MLParser::skipColor(const char*&ml) const {
        if (get(ml) != '#') return ERROR_FALSE(ml, "expecting '#' for color");
        ++ml;
        // 6 or 8 hexadecimal digits followed by non alnum
        char c;
        const char* start(ml);
        while (true) {
            c = get(ml);
            if (!isxdigit(c)) break;
            ++ml;
        }
        if (isalpha(c)) return ERROR_FALSE(ml, "non-hexa digit in color");
        if (ml - start != 6 && ml - start != 8) return ERROR_FALSE(ml, "expecting 6 or 8 hex digits in color");
        return true;
    }

    int MLParser::parseString(const char*&ml, int prev) {
        prev = newEntry(EntryType::String, ml, prev);
        if (!skipString(ml)) return -1;
        return prev;
    }

    bool MLParser::skipString(const char*&ml) const {
        if (*ml != '"' && *ml != '\'') return ERROR_FALSE(ml, "expecting '\"' or '\'' for string");
        char end(*ml);
        ++ml;
        while (true) {
            char c = get(ml);
            if (c == end) { ++ml; return true; }
            if (!c) return ERROR_FALSE(ml, "expection '\"' or '\'' but found EOF");
            ++ml;
        }
    }

    int MLParser::parseId(const char*&ml, int prev) {
        auto c(get(ml));
        if (!isalpha(c) && c != '_') return ERROR_INT(ml, "expecting id");
        prev = newEntry(EntryType::Id, ml++, prev);
        skipId(ml);
        return prev;
    }

    void MLParser::skipId(const char*&ml) const {
        char c;
        while (c = get(ml), isalnum(c) || c == '_') ++ml;
    }

    char MLParser::skipSpace(const char*&ml) const {
        while (true) {
            char c(get(ml));
            if (c == '\n') { DIAG(line++); ++ml; }
            else if (isspace(c)) ++ml;
            else if (c == '/' && get(ml + 1) == '/') skipLine(ml); // comment
            else return c;
        }
    }

    void MLParser::skipLine(const char*& ml) const {
        while (true) {
            char c(get(ml));
            if (c == '\n') { DIAG(line++); ++ml; break; }
            if (!c) break;
            ++ml;
        }
    }

    int MLParser::newEntry(EntryType type, const char* pos, int prev) {
        if (prev >= 0) entries[prev].next = entries.size();
        entries.push_back(Entry(type, pos DIAG(, line)));
        return entries.size() - 1;
    }

    int MLParser::size(int iEntry) const {
        assert(iEntry < int(entries.size()));
        auto& entry(entries[iEntry]);
        const char* end(entry.pos);
        switch (entry.type()) {
        case EntryType::Unknown:   return 0;
        case EntryType::Id:
        case EntryType::Object:
        case EntryType::Attribute:
        case EntryType::Function:  skipId(end); break;
        case EntryType::Number:    skipNumber(end); break;
        case EntryType::Color:     skipColor(end); break;
        case EntryType::String:    skipString(end); break;
        case EntryType::List:                    // '['
        case EntryType::Block:                   // '{'
        case EntryType::Wildcar:   ++end; break; // '@'
        case EntryType::Operator:  end += isOperator(end); break;
        default:                   abort();
        }
        return end - entry.pos;
    }

    int MLParser::isOperator(const char* c) {
        if (*c == '+' || *c == '-' || *c == '*' || *c == '/' || *c == '=' || *c == '%') return 1;
        if (*c == '^') {
            if (c[1] == '=') return 2;
        }
        return 0;
    }

    Identifier MLParser::asId(int iEntry) const {
        return Identifier(Context::strMng.search(entries[iEntry].pos, size(iEntry)).getId());
    }

    Identifier MLParser::asIdAdd(int iEntry) const {
        return Identifier(Context::strMng.add(entries[iEntry].pos, size(iEntry)).getId());
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
        if (iEntry < jEntry) {
            int size( (jEntry < int(entries.size()) ? entries[jEntry].pos : mlEnd) - entries[iEntry].pos );
            dst.mlOrig = strndup(entries[iEntry].pos, size);
            dst.mlEnd = dst.mlOrig + size;
            dst.ownOrig = true;
            dst.entries.assign(entries.begin() + iEntry, entries.begin() + jEntry);
            // fix next indices and text positions in dst parser
            for (auto& entry: dst.entries) {
                entry.pos += dst.mlOrig - entries[iEntry].pos;
                entry.next = entry.next ? entry.next - iEntry : 0;
            }
        }
    }

    void MLParser::swap(MLParser& o) {
        ::swap(ownOrig, o.ownOrig);
        ::swap(mlOrig, o.mlOrig);
        ::swap(mlEnd, o.mlEnd);
        DIAG(::swap(line, o.line));
        entries.swap(o.entries);
    }

    DIAG(
        bool MLParser::error(const char* ml, const char* msg, int line_) const {
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
                LOG(CYAN "%*s%d" RESET " %.*s " RED "%s " BLUE "%d" RESET,
                    level*2, "", iEntry, size(iEntry), entry.pos, toString(entry.type()), entry.next);
                if (next > iEntry + 1) // children
                    dumpTreeRecur(iEntry + 1, next, level + 1);
                iEntry = next;
            }
        });

}
