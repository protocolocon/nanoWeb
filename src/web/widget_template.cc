/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_template.h"
#include "context.h"

using namespace std;
using namespace webui;

namespace {

    TypeWidget widgetTemplateType = {
        Identifier::Template, sizeof(WidgetTemplate), {
            { Identifier::InvalidId,      PROP(WidgetTemplate, parser,  Parser,        1, 0, 0) },
        }
    };

}

namespace webui {

    WidgetTemplate::WidgetTemplate(Widget* parent): Widget(parent) {
        typeWidget = &widgetTemplateType;
    }

    TypeWidget& WidgetTemplate::getType() {
        return widgetTemplateType;
    }

    bool WidgetTemplate::setData(int iTpl, int fTpl) {
        return Context::app.updateTemplate(this, iTpl, fTpl);
    }

}
