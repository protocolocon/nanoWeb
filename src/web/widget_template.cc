/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "widget_template.h"

using namespace std;
using namespace webui;

namespace {

    TypeWidget widgetTemplateType = {
        Identifier::Template, sizeof(WidgetTemplate), { }
    };

}

namespace webui {

    WidgetTemplate::WidgetTemplate(Widget* parent): Widget(parent) {
        visible = false;
        typeWidget = &widgetTemplateType;
    }

    TypeWidget& WidgetTemplate::getType() {
        return widgetTemplateType;
    }

}
