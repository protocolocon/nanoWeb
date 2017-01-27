/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "action.h"
#include "ml_parser.h"
#include "type_widget.h"

using namespace webui;

namespace {

    DIAG(const char* toString(TypeBase type) {
            static const char* typeNames[] = {
                "float",
                "id",
                "string",
                "color",
            };
            return typeNames[int(type)];
        });

}

namespace webui {

    Actions::Actions(StringManager& strMng, WidgetMap& widgets):
        actions(1, Command(Instruction::Return)),
        strMng(strMng),
        widgets(widgets) {
    }

    int Actions::add(MLParser& parser, int iEntry, int fEntry, Widget* widget) {
        if (parser[iEntry].type() == MLParser::EntryType::List) iEntry++; // list
        int dev(actions.size());

        while (iEntry < fEntry) {
            // function, expression or assignment expected
            const auto& entry(parser[iEntry]);
            switch (entry.type()) {
            case MLParser::EntryType::Number:
                // just push it
                actions.push_back(Command(Instruction::PushFloatConstant));
                actions.push_back(Command(atof(entry.pos)));
                break;
            case MLParser::EntryType::Id:
                // resolve the symbol
                if (!pushSymbol(parser, iEntry, widget)) return 0;
            default:
                break;
            }
            iEntry = entry.next;
        }
        actions.push_back(Command(Instruction::Return));
        return dev;
    }

    bool Actions::pushSymbol(MLParser& parser, int iEntry, Widget* widget) {
        const auto& entry(parser[iEntry]);
        int size(parser.size(iEntry));
        int wordStart(0);
        for (int i = 0; i < size; i++) {
            if (*(entry.pos + i) == '.') {
                wordStart = i + 1;
                abort(); // TODO: widget addressing
            }
        }
        // last part, property
        auto id(strMng.search(entry.pos + wordStart, size - wordStart));
        if (!id.valid()) {
            DIAG(LOG("identifier '%.*s' is not registered", size - wordStart, entry.pos + wordStart));
            return false;
        }
        return true;
    }

    DIAG(void Actions::dump(int i) const {
            LOG("actions entry: %d", i);
            while (true) {
                Command com(actions[i]);
                switch (com.inst()) {
                case Instruction::Return:
                    LOG("%6d " GREEN "%-20s" RESET, i, "Return");
                    return;
                case Instruction::PushFloatConstant:
                    LOG("%6d " GREEN "%-20s " RESET ": %12.4f", i, "Push float", actions[i + 1].f);
                    i++;
                    break;
                case Instruction::PushProperty:
                    LOG("%6d " GREEN "%-20s " RESET ": type(%s), offset(%d) ",
                        i, "Push prop", ::toString(TypeBase(actions[i].sub)), actions[i].param);
                default:
                    LOG("%6d " RED "%-20s" RESET, i, "internal error");
                    break;
                }
                i++;
            }
        });

}
