/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "input.h"
#include "widget.h"
#include "context.h"
#include "widget_timer.h"
#include "widget_layout.h"
#include "widget_template.h"
#include "widget_application.h"
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <algorithm>

using namespace std;
using namespace webui;

namespace {

    const Type logParams[] =             { Type::StrId, Type::LastType };
    const Type toggleVisibleParams[] =   { Type::StrId, Type::LastType };
    const Type beginPathParams[] =       { Type::LastType };
    const Type movetoParams[] =          { Type::Coord, Type::Coord, Type::LastType };
    const Type linetoParams[] =          { Type::Coord, Type::Coord, Type::LastType };
    const Type beziertoParams[] =        { Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::LastType };
    const Type closePathParams[] =       { Type::LastType };
    const Type roundedRectParams[] =     { Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::Coord, Type::LastType };
    const Type fillColorParams[] =       { Type::ColorModif, Type::LastType };
    const Type fillVertGradParams[] =    { Type::Coord, Type::Coord, Type::ColorModif, Type::ColorModif, Type::LastType };
    const Type fillParams[] =            { Type::LastType };
    const Type strokeWidthParams[] =     { Type::Float, Type::LastType };
    const Type strokeColorParams[] =     { Type::ColorModif, Type::LastType };
    const Type strokeParams[] =          { Type::LastType };
    const Type fontParams[] =            { Type::FontIdx, Type::Float, Type::LastType };
    const Type textParams[] =            { Type::Coord, Type::Coord, Type::TextPropOrStrId, Type::LastType };
    const Type translateCenterParams[] = { Type::LastType };
    const Type scale100Params[] =        { Type::Coord, Type::LastType };
    const Type resetTransformParams[] =  { Type::LastType };
    const Type setParams[] =             { Type::StrId, Type::Id, Type::Str, Type::LastType };
    const Type queryParams[] =           { Type::StrId, Type::StrId, Type::LastType };

    template <typename T>
    inline void* getPtr(int objectSize) {
        size_t size(objectSize ? objectSize : sizeof(T));
        //DIAG(LOG("object memory: %zu", size));
        void* ptr(malloc(size));
        memset(ptr, 0, size); // so that default value of properties is reset
        return ptr;
    }

}

namespace webui {

    Application::Application(Context& ctx): ctx(ctx), iTpl(0), fTpl(0), layoutStable(false),
                                            actionTables(1), actionCommands(1, int(Identifier::CLast)), root(nullptr),
                                            fontValid(false) {
        addReservedWords(strMng);
    }

    Application::~Application() {
        clear();
        for (auto& font: fonts)
            free(font.second);
    }

    void Application::initialize() {
        // widget inheritance
        auto begin(Widget::getType().begin());
        auto end(Widget::getType().end());
        WidgetTimer::getType().insert(begin, end);
        WidgetLayout::getTypeHor().insert(begin, end);
        WidgetLayout::getTypeVer().insert(begin, end);
        WidgetTemplate::getType().insert(begin, end);
        WidgetApplication::getType().insert(begin, end);

        new RequestXHR(*this, RequestXHR::TypeApplication, StringId(), "application.ml");
    }

    void Application::refresh() {
        if (root && ((Input::refresh(ctx.getRender().getWin()) | refreshTimers()) || !layoutStable)) {
            layoutStable = root->layout(ctx, V2s(0, 0), V2s(ctx.getRender().getWidth(), ctx.getRender().getHeight()));
            ctx.forceRender();
        }
    }

    bool Application::refreshTimers() {
        bool dev(false);
        for (auto child: root->getChildren())
            if (child->type() == Identifier::Timer)
                dev |= reinterpret_cast<WidgetTimer*>(child)->refreshTimer(ctx);
        return dev;
    }

    bool Application::update() {
        assert(root);
        return root->update(*this);
    }

    void Application::clear() {
        root = nullptr;
        // delete new types
        for (auto& widget: widgets) {
            auto* type(widget.second->typeWidget);
            if (type && type->type > Identifier::WLast) {
                // dynamic type
                for (auto& widget: widgets)
                    if (widget.second->typeWidget == type)
                        widget.second->typeWidget = nullptr;
                delete type;
            }
        }

        for (auto& widget: widgets)
            delete widget.second;
        widgets.clear();
    }

    void Application::resize(int width, int height) {
        if (root) {
            ctx.resetRatio();
            root->layout(ctx, V2s(0, 0), V2s(ctx.getRender().getWidth(), ctx.getRender().getHeight()));
        }
    }

    void Application::render() {
        if (root) {
            auto& render(ctx.getRender());
            render.beginFrame();
            root->render(ctx, 0x100);
            ctx.getRender().endFrame();
        }
    }

    void Application::onLoad(RequestXHR* xhr) {
        switch (xhr->getType()) {
        case RequestXHR::TypeApplication:
            assert(!root);
            if (!tree.parse(xhr->getData(), xhr->getNData()) || !(root = initializeConstruct())) {
                DIAG(
                    xhr->makeCString();
                    LOG("cannot parse: %s", xhr->getData()));
                clear();
            } else
                resize(ctx.getRender().getWidth(), ctx.getRender().getHeight());

            DIAG(
                //tree.dumpTree();
                //dump()
            );
            tree.clear();
            ctx.forceRender();
            break;
        case RequestXHR::TypeTemplate: {
            // get template widget
            auto it(widgets.find(xhr->getId()));
            DIAG(
                if (it == widgets.end())
                    LOG("internal: cannot find template %s", strMng.get(xhr->getId())));
            auto* tplWidget(it->second);
            tree.swap(reinterpret_cast<WidgetTemplate*>(tplWidget)->getParser());
            bool define;
            if (!tpl.parse(xhr->getData(), xhr->getNData(), true/*value*/) ||
                (iTpl = 0, fTpl = tpl.size(), !(tplWidget = initializeConstruct(tplWidget, 0, tree.size(), define, true)))) {
                LOG("error: update template");
            } else {
                tplWidget->setVisible(true);
                DIAG(
                    tree.dump();
                    tree.dumpTree();
                    LOG("----------");
                    tpl.dump();
                    tpl.dumpTree();
                    LOG("----------");
                    dump());
            }
            tree.swap(reinterpret_cast<WidgetTemplate*>(tplWidget)->getParser());
            layoutStable = false;
            ctx.forceRender();
            break;
        }
        case RequestXHR::TypeFont: {
            // check that font id matches in font list
            bool ok(false);
            StringId id(xhr->getId());
            for (auto& font: fonts)
                if (font.first == id) {
                    assert(!font.second);
                    // need to copy the font buffer
                    font.second = (char*)malloc(xhr->getNData());
                    memcpy(font.second, xhr->getData(), xhr->getNData());
                    int vgId(ctx.getRender().loadFont(strMng.get(id), font.second, xhr->getNData()));
                    if (vgId == &font - fonts.data())
                        ok = true;
                    break;
                }
            if (!ok)
                LOG("internal error: font indices: %s", strMng.get(id));
            ctx.forceRender();
            break;
        }
        default:
            DIAG(
                xhr->makeCString();
                LOG("internal error: unknown query: %s %s", strMng.get(xhr->getId()), xhr->getData()));
            break;
        }
        delete xhr;
    }

    void Application::onError(RequestXHR* xhr) {
        LOG("lost query: %s", strMng.get(xhr->getId()));
        delete xhr;
    }

    Widget* Application::initializeConstruct() {
        if (tree.size() > 1 && tree[0].next == 1 && tree[1].next == 0 && *tree[1].pos == '{') {
            bool define;
            auto* widget(createWidget(tree[0].asId(tree, strMng), nullptr));
            iTpl = fTpl = -1;
            if (widget && (widget = initializeConstruct(widget, 2, tree.size(), define, true)) && registerWidget(widget))
                return widget;
        }
        return nullptr;
    }

    Widget* Application::initializeConstruct(Widget* widget, int iEntry, int fEntry, bool& define, bool recurse) {
        define = false;
        auto iEntryOrig(iEntry);
        while (iEntry < fEntry) {
            // cases:
            //   1. key : value
            //   2. id { }
            //   3. { }
            Identifier key;
            const auto& entryKey(tree[iEntry]);
            MLParser::Entry* entryVal(nullptr);
            int next;
            if (*entryKey.pos == '{') { // case 3
                // template loop
                key = Identifier::WLast;
                next = entryKey.next ? entryKey.next : fEntry;
            } else { // cases 1 & 2
                assert(iEntry + 1 < fEntry && entryKey.next == iEntry + 1);
                entryVal = &tree[iEntry + 1];
                key = entryKey.asId(tree, strMng);
                next = entryVal->next ? entryVal->next : fEntry;
            }

            if (key == Identifier::InvalidId) { // invalid case
                DIAG(
                    auto ss(entryKey.asStrSize(tree, true));
                    LOG("error: invalid identifier {%.*s}", ss.second, ss.first);
                    tree.error(entryKey.pos, "=>", entryKey.line));
            } else if (key == Identifier::WLast) { // case 3
                if (recurse) {
                    // template iteration
                    int fTplSave(fTpl), iIter(iTpl), fIter(iTpl);
                    if (iTpl >= 0) {
                        if (iTpl < fTpl) {
                            if (*tpl[iTpl].pos == '[') { // list of lists
                                iIter = iTpl + 1;
                                fIter = tpl[iTpl].next ? tpl[iTpl].next : fTpl;
                                iTpl = fIter; // skip the list of lists
                            } else {
                                DIAG(
                                    LOG("expected list of lists");
                                    tpl.error(tpl[iTpl].pos, "=>", tpl[iTpl].line));
                            }
                        } else {
                            DIAG(LOG("run out of template values while preparing template loop %d %d", iTpl, fTpl));
                            delete widget;
                            return nullptr;
                        }
                    } else
                        DIAG(LOG("error: template iteration, but no template data"));

                    //LOG("ITERATION: %d %d", iIter, fIter);
                    while (iIter < fIter) {
                        // template iteration list (widget)
                        int iWidget(-1), fWidget(-1);
                        assert(iIter >= 0);
                        if (*tpl[iIter].pos == '[') { // widget
                            iWidget = iIter + 1;
                            fWidget = tpl[iIter].next ? tpl[iIter].next : fIter;
                        } else {
                            DIAG(
                                LOG("expected widget []");
                                tpl.error(tpl[iIter].pos, "=>", tpl[iIter].line));
                        }
                        bool defineChild(false);
                        //LOG("   WIDGET: %d %d", iWidget, fWidget);
                        iTpl = iWidget;
                        fTpl = fWidget;
                        if (!(widget = initializeConstruct(widget, iEntry + 1, next, defineChild, true))) {
                            DIAG(LOG("template loop initialize construct"));
                            delete widget;
                            return nullptr;
                        }
                        DIAG(if (defineChild) LOG("error: definition inside loop"));
                        DIAG(
                            if (iTpl < fTpl)
                                LOG("trailing template values after widget: %d %d", iTpl, fTpl));
                        // prepare next template list (widget)
                        iIter = fWidget; // next widget
                    }
                    iTpl = fIter;
                    fTpl = fTplSave;
                }
            } else if (isWidget(key)) { // case 2
                if (recurse) {
                    // child widget
                    Widget* widgetChild(createWidget(key, widget/*parent*/));
                    assert(widgetChild);
                    bool defineChild(false);
                    bool isTemplate(key == Identifier::Template);
                    if (!(widgetChild = initializeConstruct(widgetChild, iEntry + 2, next, defineChild, !isTemplate)) ||
                        !registerWidget(widgetChild)) {
                        delete widgetChild;
                        return nullptr;
                    }
                    if (!defineChild) widget->addChild(widgetChild);
                    if (isTemplate) {
                        auto* tpl(reinterpret_cast<WidgetTemplate*>(widgetChild));
                        tree.copyTo(tpl->getParser(), iEntry + 2, next);
                    }
                }
            } else { // case 1
                // check for @
                bool templateReplaced(replaceProperty(iEntry + 1));
                if (key == Identifier::define) {
                    key = Identifier::id;
                    DIAG(if (define) LOG("warning: repeated 'define' attribute inside widget"));
                    define = true;
                    // create new type
                    auto newTypeId(Identifier(entryAsStrId(iEntry + 1, false).getId()));
                    if (!(widget = createType(widget, newTypeId, iEntryOrig, fEntry))) {
                        DIAG(LOG("cannot create new type: %s", strMng.get(newTypeId)));
                    }
                }
                if (key != Identifier::propInt16 &&
                    key != Identifier::propText &&
                    key != Identifier::propColor &&
                    !setProp(key, widget, iEntry + 1, next)) {
                    DIAG(
                        auto ss(entryVal->asStrSize(tree, true));
                        LOG("warning: unknown attribute %s with value %.*s", strMng.get(StringId(int(key))), ss.second, ss.first);
                        tree.error(entryKey.pos, "=>", entryKey.line));
                }
                replaceBackProperty(templateReplaced, iEntry + 1);
            }
            iEntry = next;
        }
        return widget;
    }

    Widget* Application::createType(Widget* widget, Identifier typeId, int iEntry, int fEntry) {
        TypeWidget* type = new TypeWidget(typeId, widget->typeSize(), { });
        type->insert(widget->typeWidget->begin(), widget->typeWidget->end());
        // add new properties
        while (iEntry < fEntry) {
            const auto& entryKey(tree[iEntry]);
            if (*entryKey.pos == '{') { // case 3: template loop
                iEntry = entryKey.next ? entryKey.next : fEntry;
            } else { // cases 1 & 2: key: value or class {
                assert(iEntry + 1 < fEntry && entryKey.next == iEntry + 1);
                Property prop;
                prop.all = 0;
                Identifier key = entryKey.asId(tree, strMng);
                if (key == Identifier::propInt16) {
                    prop.type = Type::Int16;
                    prop.size = 2;
                } else if (key == Identifier::propText) {
                    prop.type = Type::Text;
                    prop.size = sizeof(void*);
                } else if (key == Identifier::propColor) {
                    prop.type = Type::Color;
                    prop.size = 4;
                }

                if (prop.size) {
                    // property value position and type size
                    type->size = (type->size + prop.size - 1) & -prop.size; // align to property size
                    prop.pos = type->size / prop.size;                      // store pos in size units
                    type->size += prop.size;                                // increase size
                    auto propName(entryAsStrId(iEntry + 1, false));
                    auto propId(Identifier(propName.getId()));
                    type->insert(make_pair(propId, prop));
                    DIAG(LOG("adding property: %s.%s at %d+%d of type %s",
                             strMng.get(typeId), strMng.get(propName), prop.pos * prop.size, prop.size, toString(prop.type)));
                }
                const auto& entryVal(tree[iEntry + 1]);
                iEntry = entryVal.next ? entryVal.next : fEntry;
            }
        }
        // create new widget with correct size to hold new properties
        // get base widget
        auto* widgetNew(createWidget(widget->baseType(), widget->parent, type->size));
        widgetNew->typeWidget = widget->typeWidget;
        widgetNew->copyFrom(widget);
        widgetNew->typeWidget = type;
        delete widget;
        return widgetNew;
    }

    bool Application::replaceProperty(int iEntry) {
        auto& entry(tree[iEntry]);
        if (entry.pos[0] == '@') {
            if (iTpl < fTpl) {
                swap(entry.pos, tpl[iTpl].pos);
                tree.swapEnd(tpl);
                return true;
            } else {
                DIAG(
                    if (iTpl != -1 || fTpl != -1) {
                        LOG("out of template values: %d %d", iTpl, fTpl);
                        tree.error(entry.pos, "=>", entry.line);
                    });
            }
        }
        return false;
    }

    void Application::replaceBackProperty(bool templateReplaced, int iEntry) {
        if (templateReplaced) {
            swap(tree[iEntry].pos, tpl[iTpl].pos);
            tree.swapEnd(tpl);
            iTpl++;
        }
    }

    bool Application::isWidget(Identifier id) const {
        return id < Identifier::WLast || widgets.find(int(id)) != widgets.end();
    }

    Widget* Application::createWidget(Identifier id, Widget* parent, int objectSize) {
        switch (id) {
        case Identifier::Application: return new (getPtr<WidgetApplication>(objectSize)) WidgetApplication(parent);
        case Identifier::Widget:      return new (getPtr<Widget>(objectSize)) Widget(parent);
        case Identifier::LayoutHor:   return new (getPtr<WidgetLayout>(objectSize)) WidgetLayout(parent, 0);
        case Identifier::LayoutVer:   return new (getPtr<WidgetLayout>(objectSize)) WidgetLayout(parent, 1);
        case Identifier::Template:    return new (getPtr<WidgetTemplate>(objectSize)) WidgetTemplate(parent);
        case Identifier::Timer:       return new (getPtr<WidgetTimer>(objectSize)) WidgetTimer(parent);
        default:                      break;
        }
        // copy from other widget by id
        auto it(widgets.find(int(id)));
        if (it != widgets.end()) {
            auto* widget(createWidget(it->second->baseType(), parent, it->second->typeSize()));
            widget->typeWidget = it->second->typeWidget;
            widget->copyFrom(it->second);
            return widget;
        }
        return nullptr;
    }

    bool Application::registerWidget(Widget* widget) {
        if (!widget->getId().valid()) {
            // set an internal id
            static int id(0);
            char buffer[16];
            int nBuffer(snprintf(buffer, sizeof(buffer), "@%x", id++));
            widget->setId(strMng.add(buffer, nBuffer));
        }
        const auto& id(widget->getId());
        if (widgets.count(id)) {
            DIAG(LOG("repeated widget id: %s", strMng.get(id)));
            return false;
        }
        widgets[id] = widget;
        return true;
    }

    bool Application::setProp(Identifier id, Widget* widget, int iEntry, int fEntry) {
        const auto* prop(widget->getProp(id));
        if (!prop) {
            DIAG(
                LOG("invalid property '%s' for widget of type: %s", strMng.get(id), strMng.get(widget->type()));
                widget->dump(strMng, -1, true));
            return false;
        }
        if (!setProp(*prop, id, widget, iEntry, fEntry, widget)) {
            DIAG(
                auto ss(entryAsStrSize(iEntry, true));
                LOG("%s was expecting the type %s, received %.*s (%d parameters) and failed",
                    strMng.get(id), toString(prop->type), ss.second, ss.first, fEntry - iEntry));
            return false;
        }
        return true;
    }

    bool Application::setProp(const Property& prop, Identifier id, void* data, int iEntry, int fEntry, Widget* widget) {
        pair<const char*, int> ss;
        switch (prop.type) {
        case Type::Bit:
            DIAG(if (fEntry > iEntry + 1) return false);
            if (*tree[iEntry].pos == '0')
                reinterpret_cast<uint8_t*>(data)[prop.pos] &= ~(1 << prop.bit);
            else
                reinterpret_cast<uint8_t*>(data)[prop.pos] |= 1 << prop.bit;
            return true;
        case Type::Uint8:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<uint8_t*>(data)[prop.pos] = atoi(tree[iEntry].pos);
            return true;
        case Type::Int16:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<int16_t*>(data)[prop.pos] = atoi(tree[iEntry].pos);
            return true;
        case Type::Int32:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<int*>(data)[prop.pos] = atoi(tree[iEntry].pos);
            return true;
        case Type::Id:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<Identifier*>(data)[prop.pos] = entryId(iEntry);
            return true;
        case Type::Str:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<StringId*>(data)[prop.pos] = entryAsStrId(iEntry, true);
            return true;
        case Type::StrId:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<StringId*>(data)[prop.pos] = entryAsStrId(iEntry, false);
            return true;
        case Type::Float:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<float*>(data)[prop.pos] = strtof(tree[iEntry].pos, nullptr);
            return true;
        case Type::Color:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<RGBA*>(data)[prop.pos] = entry(iEntry).pos;
            return true;
        case Type::ColorModif:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<RGBAref*>(data)[prop.pos] = parseColorModif(iEntry, widget);
            return true;
        case Type::SizeRelative:
            DIAG(if (fEntry > iEntry + 1) return false);
            ss = entryAsStrSize(iEntry, false);
            reinterpret_cast<SizeRelative*>(data)[prop.pos] = ss;
            return true;
        case Type::Coord:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<int*>(data)[prop.pos] = parseCoord(iEntry, widget);
            return true;
        case Type::FontIdx:
            DIAG(if (fEntry > iEntry + 1) return false);
            reinterpret_cast<int*>(data)[prop.pos] = getFont(entryAsStrId(iEntry, false));
            return true;
        case Type::ActionTable:
            return addAction(id, iEntry, fEntry, reinterpret_cast<int*>(data)[prop.pos], widget);
        case Type::Text:
            DIAG(if (fEntry > iEntry + 1) return false);
            ss = entryAsStrSize(iEntry, false);
            free(reinterpret_cast<char**>(data)[prop.pos]);
            reinterpret_cast<char**>(data)[prop.pos] = strndup(ss.first, ss.second);
            return true;
        case Type::TextPropOrStrId:
            DIAG(if (fEntry > iEntry + 1) return false);
            ss = entryAsStrSize(iEntry, true);
            if (*ss.first == '"' || *ss.first == '\'')
                reinterpret_cast<TextPropOrStrId*>(data)[prop.pos] = strMng.add(ss.first + 1, ss.second - 2);
            else {
                const auto* propWidget(widget->getProp(entryId(iEntry)));
                if (!propWidget) return false;
                reinterpret_cast<TextPropOrStrId*>(data)[prop.pos] = propWidget->pos;
            }
            return true;
        default:
            DIAG(LOG("internal error, unhandled property type!"));
            return false;
        }
    }

    bool Application::addAction(Identifier actionId, int iEntry, int fEntry, int& actions, Widget* widget) {
        if (!actions || widget->isSharingActions()) {
            auto actionsPrev(actions);
            actions = int(actionTables.size());
            actionTables.resize(actionTables.size() + 1);
            widget->resetSharingActions();
            if (actionsPrev)
                actionTables[actions] = actionTables[actionsPrev];
        }
        auto& table(actionTables[actions]);
        int* tableEntry;
        switch (actionId) { // get the table entry to add commands to
        case Identifier::onEnter:
        case Identifier::onTimeout:      tableEntry = &table.onEnter;        break;
        case Identifier::onLeave:        tableEntry = &table.onLeave;        break;
        case Identifier::onClick:        tableEntry = &table.onClick;        break;
        case Identifier::onRender:       tableEntry = &table.onRender;       break;
        case Identifier::onRenderActive: tableEntry = &table.onRenderActive; break;
        default: DIAG(LOG("unknown action")); return false;
        }
        return addActionCommands(iEntry, fEntry, *tableEntry, widget);
    }

    bool Application::addActionCommands(int iEntry, int fEntry, int& tableEntry, Widget* widget) {
        if (*tree[iEntry].pos == '[') iEntry++;
        bool ok(true);
        tableEntry = int(actionCommands.size());
        while (iEntry < fEntry) {
            const auto& entry(tree[iEntry++]);
            auto command(entry.asId(tree, strMng));
            int next(entry.next ? entry.next : fEntry);

            const Type* params(nullptr);
            switch (command) {
            case Identifier::log:             params = logParams; break;
            case Identifier::toggleVisible:   params = toggleVisibleParams; break;
            case Identifier::beginPath:       params = beginPathParams; break;
            case Identifier::moveto:          params = movetoParams; break;
            case Identifier::lineto:          params = linetoParams; break;
            case Identifier::bezierto:        params = beziertoParams; break;
            case Identifier::closePath:       params = closePathParams; break;
            case Identifier::roundedRect:     params = roundedRectParams; break;
            case Identifier::fillColor:       params = fillColorParams; break;
            case Identifier::fillVertGrad:    params = fillVertGradParams; break;
            case Identifier::fill:            params = fillParams; break;
            case Identifier::strokeWidth:     params = strokeWidthParams; break;
            case Identifier::strokeColor:     params = strokeColorParams; break;
            case Identifier::stroke:          params = strokeParams; break;
            case Identifier::font:            params = fontParams; break;
            case Identifier::text:
            case Identifier::textLeft:        params = textParams; break;
            case Identifier::translateCenter: params = translateCenterParams; break;
            case Identifier::scale100:        params = scale100Params; break;
            case Identifier::resetTransform:  params = resetTransformParams; break;
            case Identifier::set:             params = setParams; break;
            case Identifier::query:           params = queryParams; break;
            default:
                DIAG(
                    auto ss(entry.asStrSize(tree, true));
                    LOG("unknown command: {%.*s}", ss.second, ss.first));
            }
            if (params) ok &= addCommandGeneric(command, iEntry, next, params, widget);
            iEntry = next;
        }
        actionCommands.push_back(int(Identifier::CLast));
        return ok;
    }

    bool Application::addCommandGeneric(Identifier name, int iEntry, int fEntry, const Type* params, Widget* widget) {
        DIAG(
            if (params[fEntry - iEntry] != Type::LastType) {
                LOG("%s command got %d parameters", strMng.get(name), fEntry - iEntry);
                return false;
            });
        actionCommands.push_back(int(name));
        Property prop;
        prop.all = 0;
        prop.size = 4;
        while (*params != Type::LastType) {
            int value;
            prop.type = *params++;
            bool templateReplaced(replaceProperty(iEntry));
            if (!setProp(prop, Identifier::InvalidId, &value, iEntry, iEntry + 1, widget))
                DIAG(LOG("error in command argument: %d", int(prop.type)));
            replaceBackProperty(templateReplaced, iEntry);
            iEntry++;
            actionCommands.push_back(value);
        }
        return true;
    }

    bool Application::parseId(Widget* widget, const char*& str, const Property*& prop) const {
        if (isalpha(*str)) {
            const auto* start(str++);
            while (isalnum(*str)) ++str;
            auto id(strMng.search(start, str - start));
            if (id.valid()) return (prop = widget->getProp(Identifier(id.getId())));
            return false; // error
        }
        prop = nullptr;
        return true; // ok
    }

    int Application::parseCoord(int iEntry, Widget* widget) const {
        DIAG(bool bad(false));
        int out(0);
        auto param(entry(iEntry).pos);
        const Property* prop(nullptr);
        if (!parseId(widget, param, prop)) DIAG(bad = true);
        if (prop) {
            if (prop->size == 2 && prop->type == Type::Int16) out |= prop->pos << 16;
            else DIAG(bad = true);
        } else
            out |= 0xffff0000;
        if (*param == '+' || *param == '-' || isdigit(*param)) {
            float num(strtof(param, nullptr));
            out |= int(num*4) & 0xffff;
        }
        DIAG(
            if (bad) {
                auto ss(entryAsStrSize(iEntry, true));
                LOG("unrecognized coord string: %.*s", ss.second, ss.first);
            });
        return out;
    }

    RGBAref Application::parseColorModif(int iEntry, Widget* widget) const {
        auto param(entry(iEntry).pos);
        if (*param == '"') return RGBAref(param); // normal color, no reference
        // referenced color
        const Property* prop;
        if (!parseId(widget, param, prop) DIAG(|| !prop || prop->size != 4 || prop->type != Type::Color)) {
            DIAG(
                auto ss(entryAsStrSize(iEntry, true));
                LOG("unrecognized color string: %.*s", ss.second, ss.first));
            return RGBAref();
        } else {
            auto ref(prop->pos);
            float value(100);
            if (*param == '%') value = strtof(param + 1, nullptr);
            return RGBAref(ref, value);
        }
    }

    int Application::getFont(StringId str) {
        for (auto& font: fonts)
            if (font.first == str) return &font - fonts.data();
        // add font
        fonts.push_back(make_pair(str, nullptr));
        new RequestXHR(*this, RequestXHR::TypeFont, str, strMng.get(str));
        return fonts.size() - 1;
    }

    bool Application::executeNoCheck(int commandList, Widget* w) {
        bool cont(true), executed(false);
        V2s pos;
        float value;
        auto& render(ctx.getRender());
        while (cont) {
            auto* const command(&actionCommands[commandList]);
            switch (Identifier(*command)) {
            case Identifier::log:
                LOG("%s", strMng.get(StringId(command[1])));
                commandList += 2;
                break;
            case Identifier::toggleVisible:
                executed |= executeToggleVisible(StringId(command[1]));
                commandList += 2;
                break;
            case Identifier::beginPath:
                render.beginPath();
                ++commandList;
                break;
            case Identifier::moveto:
                render.moveto(getCoord(command[1], w), getCoord(command[2], w));
                commandList += 3;
                break;
            case Identifier::lineto:
                render.lineto(getCoord(command[1], w), getCoord(command[2], w));
                commandList += 3;
                break;
            case Identifier::bezierto:
                render.bezierto(getCoord(command[1], w), getCoord(command[2], w),
                                getCoord(command[3], w), getCoord(command[4], w),
                                getCoord(command[5], w), getCoord(command[6], w));
                commandList += 7;
                break;
            case Identifier::closePath:
                render.closePath();
                ++commandList;
                break;
            case Identifier::roundedRect:
                render.roundedRect(getCoord(command[1], w), getCoord(command[2], w), getCoord(command[3], w),
                                   getCoord(command[4], w), getCoord(command[5], w));
                commandList += 6;
                break;
            case Identifier::fillColor:
                render.fillColor(RGBAref(command[1]).get(w));
                commandList += 2;
                break;
            case Identifier::fillVertGrad:
                render.fillVertGrad(getCoord(command[1], w), getCoord(command[2], w), RGBAref(command[3]).get(w), RGBAref(command[4]).get(w));
                commandList += 5;
                break;
            case Identifier::fill:
                render.fill();
                ++commandList;
                break;
            case Identifier::strokeWidth:
                render.strokeWidth(*(float*)&command[1]);
                commandList += 2;
                break;
            case Identifier::strokeColor:
                render.strokeColor(RGBAref(command[1]).get(w));
                commandList += 2;
                break;
            case Identifier::stroke:
                render.stroke();
                ++commandList;
                break;
            case Identifier::font:
                if (command[1] < int(fonts.size()) && fonts[command[1]].second) {
                    render.font(command[1]);
                    fontValid = true;
                }
                render.fontSize(*(float*)&command[2]);
                commandList += 3;
                break;
            case Identifier::text:
                if (fontValid) {
                    render.textAlign(NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
                    pos = w->curPos + (w->curSize >> 1);
                    render.text(pos.x + getCoord(command[1], w), pos.y + getCoord(command[2], w),
                                reinterpret_cast<TextPropOrStrId*>(&command[3])->get(w, strMng));
                }
                commandList += 4;
                break;
            case Identifier::textLeft:
                if (fontValid) {
                    render.textAlign(NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
                    render.text(w->curPos.x + getCoord(command[1], w), w->curPos.y + (w->curSize.y >> 1) + getCoord(command[2], w),
                                reinterpret_cast<TextPropOrStrId*>(&command[3])->get(w, strMng));
                }
                commandList += 4;
                break;
            case Identifier::translateCenter:
                render.translate(w->curSize.x >> 1, w->curSize.y >> 1);
                ++commandList;
                break;
            case Identifier::scale100:
                value = getCoord(command[1], w) * 0.01;
                render.scale(value, value);
                commandList += 2;
                break;
            case Identifier::resetTransform:
                render.resetTransform();
                ++commandList;
                break;
            case Identifier::set:
                executed |= executeSet(StringId(command[1]), Identifier(command[2]), StringId(command[3]), w);
                commandList += 4;
                break;
            case Identifier::query:
                executed |= executeQuery(StringId(command[1]), StringId(command[2]));
                commandList += 3;
                break;
            case Identifier::CLast:
                cont = false;
                break;
            default:
                DIAG(
                    LOG("internal: executing command error: %d %d %zu", *command, commandList, actionCommands.size());
                    for (auto* command = &actionCommands[commandList]; command < &actionCommands.back(); command++)
                        LOG("%08x", *command));
                cont = false;
                break;
            }
        }
        return executed;
    }

    bool Application::executeToggleVisible(StringId widgetId) {
        auto len(getWidgetRange(widgetId));
        const char* idSearch(strMng.get(widgetId));
        bool toggled(false);
        for (auto& widget: widgets) {
            if (!memcmp(strMng.get(widget.first), idSearch, len)) {
                widget.second->toggleVisible();
                toggled = true;
            }
        }
        return toggled;
    }

    bool Application::executeSet(StringId widgetId, Identifier prop, StringId value, Widget* w) {
        bool set(false);
        auto iEntry(tree.getTemporalEntry(strMng.get(value)));
        if (widgetId.getId() == int(Identifier::self)) {
            // change own property
            if (!setProp(prop, w, iEntry, iEntry + 1))
                DIAG(LOG("error setting property"));
            set = true;
        } else {
            // change in the tree
            auto len(getWidgetRange(widgetId));
            const char* idSearch(strMng.get(widgetId));
            for (auto& widget: widgets) {
                if (!memcmp(strMng.get(widget.first), idSearch, len)) {
                    if (!setProp(prop, widget.second, iEntry, iEntry + 1))
                        DIAG(LOG("error setting property"));
                    set = true;
                }
            }
        }
        return set;
    }

    bool Application::executeQuery(StringId query, StringId widgetId) {
        // execute only in visible widgets
        auto it(widgets.find(widgetId));
        DIAG(
            if (it == widgets.end())
                LOG("internal: cannot find widget %s to send query", strMng.get(widgetId)));
        if (it->second->visible)
            new RequestXHR(*this, RequestXHR::TypeTemplate, widgetId, strMng.get(query));
        return true;
    }

    int Application::getWidgetRange(StringId widgetId) const {
        const char* id(strMng.get(widgetId));
        int len(strlen(id));
        if (len && id[len - 1] == '*') return len - 1;
        return len + 1;
    }

    DIAG(
        void Application::dump() const {
            if (root) {
                LOG("Application tree");
                root->dump(strMng);
            }
            LOG("Registered widgets:");
            for (const auto& widget: widgets)
                widget.second->dump(strMng, -1, true);
        });

}
