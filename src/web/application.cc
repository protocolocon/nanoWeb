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
#include <unordered_set>

using namespace std;
using namespace webui;

namespace {

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

    Application::Application(): iTpl(0), fTpl(0), startedTpl(false), layoutStable(false),
                                actionTables(1), root(nullptr) {
    }

    DIAG(Application::~Application() {
            clear();
            for (auto& font: fonts)
                free(font.second);
        });

    void Application::initialize() {
        DIAG(clear());
        // widget inheritance
        auto begin(Widget::getType().begin());
        auto end(Widget::getType().end());
        WidgetTimer::getType().insert(begin, end);
        WidgetLayout::getTypeHor().insert(begin, end);
        WidgetLayout::getTypeVer().insert(begin, end);
        WidgetTemplate::getType().insert(begin, end);
        WidgetApplication::getType().insert(begin, end);
    }

    void Application::refresh() {
        if (root && ((Input::refresh(Context::render.getWin()) | refreshTimers()) || !layoutStable)) {
            layoutStable = root->layout(Box4f(0.f, 0.f, float(Context::render.getWidth()), float(Context::render.getHeight())));
            ctx.forceRender();
        }
    }

    bool Application::refreshTimers() {
        bool dev(false);
        for (auto child: root->getChildren())
            if (child->type() == Identifier::Timer)
                dev |= reinterpret_cast<WidgetTimer*>(child)->refreshTimer();
        return dev;
    }

    bool Application::update() {
        assert(root);
        return root->update();
    }

    DIAG(void Application::clear() {
            root = nullptr;
            // get all new types for removal
            unordered_set<TypeWidget*> types;
            for (auto& widget: widgets) {
                auto* type(widget.second->typeWidget);
                if (type && type->type > Identifier::WLast) types.insert(type);
            }
            // delete widgets
            for (auto& widget: widgets)
                delete widget.second;
            widgets.clear();
            // delete new types
            for (auto type: types)
                delete type;
        });

    void Application::resize(int width, int height) {
        if (root) {
            ctx.resetRatio();
            root->layout(Box4f(0.f, 0.f, float(Context::render.getWidth()), float(Context::render.getHeight())));
        }
    }

    void Application::render() {
        if (root) {
            Context::render.beginFrame();
            root->render(0x100);
            Context::render.endFrame();
        }
    }

    bool Application::onLoad(RequestXHR* xhr) {
        DIAG(if (!xhr->getData() || !xhr->getNData()) { LOG("empty xhr response"); return false; });
        bool dev(true);
        switch (xhr->getType()) {
        case RequestXHR::TypeApplication:
            assert(!root);
            if (!tree.parse(xhr->getData(), xhr->getNData()) || !(root = initializeConstruct()) || !checkActions()) {
                DIAG(
                    xhr->makeCString();
                    LOG("cannot parse: %s", xhr->getData());
                    clear());
                dev = false;
            } else
                resize(Context::render.getWidth(), Context::render.getHeight());

            DIAG(
                tree.dumpTree();
                dump()
            );
            tree.clear();
            ctx.forceRender();
            break;
        case RequestXHR::TypeTemplate: {
            // get template widget
            auto it(widgets.find(xhr->getId()));
            if (it == widgets.end()) {
                DIAG(LOG("internal: cannot find template %s", Context::strMng.get(xhr->getId())));
                dev = false;
            } else {
                if (!tpl.parse(xhr->getData(), xhr->getNData()) || tpl.empty() || tpl[0].type() != MLParser::EntryType::List) {
                    DIAG(LOG("cannot parse template info or is not a complete list"));
                    dev = false;
                } else {
                    iTpl = 1;
                    fTpl = tpl[0].next;

                    auto* tplWidget(it->second);
                    tree.swap(reinterpret_cast<WidgetTemplate*>(tplWidget)->getParser());
                    DIAG(
                        tree.dumpTree();
                        LOG("----------");
                        tpl.dumpTree();
                        LOG("----------");
                        dump());
                    bool define;
                    if (!(tplWidget = initializeConstruct(tplWidget, 0, tree.size(), define, true))) {
                        LOG("error: update template");
                        dev = false;
                    } else {
                        tplWidget->setVisible(true);
                        layoutStable = false;
                        ctx.forceRender();
                    }
                    tree.swap(reinterpret_cast<WidgetTemplate*>(it->second)->getParser());
                }
            }
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
                    int vgId(Context::render.loadFont(Context::strMng.get(id), font.second, xhr->getNData()));
                    if (vgId == &font - fonts.data())
                        ok = true;
                    break;
                }
            if (!ok) {
                LOG("internal error: font indices: %s", Context::strMng.get(id));
                dev = false;
            }
            ctx.forceRender();
            break;
        }
        default:
            DIAG(
                xhr->makeCString();
                LOG("internal error: unknown query: %s %s", Context::strMng.get(xhr->getId()), xhr->getData()));
            break;
        }
        delete xhr;
        return dev;
    }

    void Application::onError(RequestXHR* xhr) {
        LOG("lost query: %s", Context::strMng.get(xhr->getId()));
        delete xhr;
    }

    Widget* Application::initializeConstruct() {
        DIAG(tree.dumpTree());
        if (!tree.empty() && tree[0].type() == MLParser::EntryType::Object) {
            bool define;
            auto* widget(createWidget(tree.asId(0), nullptr));
            iTpl = fTpl = -1;
            if (widget && (widget = initializeConstruct(widget, 1, tree.size(), define, true)) && registerWidget(widget))
                return widget;
        } DIAG(else LOG("expecting object as root of application"));
        return nullptr;
    }

    Widget* Application::initializeConstruct(Widget* widget, int iEntry, int fEntry, bool& define, bool recurse) {
        define = false;
        auto iEntryOrig(iEntry);
        while (iEntry < fEntry) {
            // cases:
            //   1. key : value
            //   2. id { }
            //   3. [ ] <- block
            auto valEntry(iEntry + 1);
            const auto& entryKey(tree[iEntry]);
            if (entryKey.type() == MLParser::EntryType::Id) { // case 1: key : value
                assert(valEntry < fEntry && tree[iEntry].next == valEntry);
                auto key(tree.asId(iEntry));
                if (key == Identifier::InvalidId) {
                    DIAG(LOG("invalid key for widget: %.*s", tree.size(iEntry), entryKey.pos));
                    return nullptr;
                }
                // new widget type definition
                if (key == Identifier::define) {
                    key = Identifier::id;
                    DIAG(if (define) LOG("warning: repeated 'define' attribute inside widget"));
                    define = true;
                    // create new type
                    auto newTypeId(tree.asIdAdd(valEntry));
                    if (!(widget = createType(widget, newTypeId, iEntryOrig, fEntry))) {
                        DIAG(LOG("cannot create new type: %s", Context::strMng.get(newTypeId)));
                        return nullptr;
                    }
                }
                if (int(key) > int(Identifier::ALast) && int(key) <= int(Identifier::PLast)) {
                    // addition of properties in widget definition
                    DIAG(if (!define) {
                            LOG("defining a property for a widget that is not being defined");
                            tree.error(entryKey.pos, "=>", entryKey.line);
                            return nullptr;
                        });
                } else if(!Context::actions.evalProperty(tree, valEntry, tree[valEntry].next, key, widget)) { // eval property
                    DIAG(
                        LOG("warning: unknown attribute %s with value %.*s", Context::strMng.get(key), tree.size(valEntry), tree[valEntry].pos);
                        tree.error(entryKey.pos, "=>", entryKey.line);
                        return nullptr);
                }
                iEntry = tree[valEntry].next;

            } else if (entryKey.type() == MLParser::EntryType::Object) { // case 2: object { }
                auto key(tree.asId(iEntry));
                DIAG(if (!isWidget(key)) {
                         LOG("expecting object id");
                         tree.error(entryKey.pos, "=>", entryKey.line);
                    });
                if (recurse) {
                    // child widget
                    Widget* widgetChild(createWidget(key, widget/*parent*/));
                    assert(widgetChild);
                    bool defineChild(false);
                    bool isTemplate(key == Identifier::Template);
                    if (!(widgetChild = initializeConstruct(widgetChild, valEntry, tree[iEntry].next, defineChild, !isTemplate)) ||
                        !registerWidget(widgetChild)) {
                        delete widgetChild;
                        return nullptr;
                    }
                    if (!defineChild) widget->addChild(widgetChild);
                    if (isTemplate) {
                        auto* tpl(reinterpret_cast<WidgetTemplate*>(widgetChild));
                        tree.copyTo(tpl->getParser(), valEntry, tree[iEntry].next);
                    }
                }
                iEntry = tree[iEntry].next;

            } else if (entryKey.type() == MLParser::EntryType::Block) { // case 3: block
                if (recurse) {
                    // template iteration
                    int fTplSave(fTpl), iIter(iTpl), fIter(iTpl);
                    if (iTpl >= 0) {
                        if (iTpl < fTpl) {
                            if (tpl[iTpl].type() == MLParser::EntryType::List) { // list of lists
                                iIter = iTpl + 1;
                                fIter = tpl[iTpl].next;
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
                        if (tpl[iIter].type() == MLParser::EntryType::List) { // widget
                            iWidget = iIter + 1;
                            fWidget = tpl[iIter].next;
                        } else {
                            DIAG(
                                LOG("expected widget []");
                                tpl.error(tpl[iIter].pos, "=>", tpl[iIter].line));
                        }
                        bool defineChild(false);
                        //LOG("   WIDGET: %d %d", iWidget, fWidget);
                        iTpl = iWidget;
                        fTpl = fWidget;
                        if (!(widget = initializeConstruct(widget, valEntry, tree[valEntry].next, defineChild, true))) {
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
                iEntry = tree[iEntry].next;
            }
        }
        return widget;
    }

    Widget* Application::createType(Widget* widget, Identifier typeId, int iEntry, int fEntry) {
        TypeWidget* type = new TypeWidget(typeId, widget->typeSize(), { });
        type->insert(widget->typeWidget->begin(), widget->typeWidget->end());
        // add new properties
        while (iEntry < fEntry) {
            const auto& entryKey(tree[iEntry]);
            if (entryKey.type() == MLParser::EntryType::Block) { // case 3: template loop
                iEntry = entryKey.next;
            } else { // cases 1 & 2: key: value or class {
                DIAG(if (iEntry + 1 >= fEntry || entryKey.next != iEntry + 1) {
                        LOG("expected key: value or class {");
                        tree.error(entryKey.pos, "=>", entryKey.line);
                        return nullptr;
                    });
                Property prop;
                prop.all = 0;
                Identifier key = tree.asId(iEntry);
                switch (key) {
                case Identifier::propInt16: prop.type = Type::Int16; prop.size = 2; break;
                case Identifier::propText:  prop.type = Type::Text;  prop.size = sizeof(void*); break;
                case Identifier::propColor: prop.type = Type::Color; prop.size = 4; break;
                case Identifier::propId:    prop.type = Type::Id;    prop.size = 4; break;
                default: break;
                }

                if (prop.size) {
                    // property value position and type size
                    type->size = (type->size + prop.size - 1) & -prop.size; // align to property size
                    prop.pos = type->size / prop.size;                      // store pos in size units
                    type->size += prop.size;                                // increase size
                    auto propId(tree.asIdAdd(iEntry + 1));
                    type->insert(make_pair(propId, prop));
                    DIAG(LOG("adding property: %s.%s at %d+%d of type %s",
                             Context::strMng.get(typeId), Context::strMng.get(propId), prop.pos * prop.size, prop.size, toString(prop.type)));
                }
                const auto& entryVal(tree[iEntry + 1]);
                iEntry = entryVal.next;
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

    bool Application::startTemplate(int& iTpl_, int& fTpl_) {
        if (startedTpl) {
            DIAG(LOG("cannot use wildcars in template or previous template failed"));
            return false;
        }
        if (iTpl >= fTpl) {
            DIAG(LOG("out of template values: %d %d", iTpl, fTpl));
            return false;
        }
        iTpl_ = iTpl;
        fTpl_ = tpl[iTpl].next;
        startedTpl = true;
        return true;
    }

    bool Application::endTemplate() {
        assert(startedTpl);
        iTpl = tpl[iTpl].next;
        startedTpl = false;
        return true;
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
            widget->setId(Context::strMng.add(buffer, nBuffer));
        }
        const auto& id(widget->getId());
        if (widgets.count(id)) {
            DIAG(LOG("repeated widget id: %s", Context::strMng.get(id)));
            return false;
        }
        widgets[id] = widget;
        return true;
    }

    bool Application::checkActions() {
        bool dev(true);
        for (auto& table: actionTables) table.checked = false;
        for (auto& idWidget: widgets) {
            auto& table(actionTables[idWidget.second->actions]);
            if (!table.checked) {
                table.checked = true;
                for (auto action: table.actions)
                    dev &= Context::actions.execute<true>(action, idWidget.second);
            }
        }
        return dev;
    }

    bool Application::addAction(Identifier actionId, int iAction, Widget* widget) {
        if (!widget->actions || widget->isSharingActions()) {
            auto widgetActionsPrev(widget->actions);
            widget->actions = int(actionTables.size());
            actionTables.resize(actionTables.size() + 1);
            widget->resetSharingActions();
            if (widgetActionsPrev)
                actionTables[widget->actions] = actionTables[widgetActionsPrev];
        }
        auto& table(actionTables[widget->actions]);
        switch (actionId) { // get the table entry to add commands to
        case Identifier::onEnter:
        case Identifier::onTimeout:      table.onEnter        = iAction; break;
        case Identifier::onLeave:        table.onLeave        = iAction; break;
        case Identifier::onClick:        table.onClick        = iAction; break;
        case Identifier::onRender:       table.onRender       = iAction; break;
        case Identifier::onRenderActive: table.onRenderActive = iAction; break;
        default: DIAG(LOG("unknown action")); return false;
        }
        return true;
    }

    bool Application::parseId(Widget* widget, const char*& str, const Property*& prop) const {
        if (isalpha(*str)) {
            const auto* start(str++);
            while (isalnum(*str)) ++str;
            auto id(Context::strMng.search(start, str - start));
            if (id.valid()) return (prop = widget->getProp(Identifier(id.getId())));
            return false; // error
        }
        prop = nullptr;
        return true; // ok
    }

    int Application::getFont(StringId str) {
        for (auto& font: fonts)
            if (font.first == str) return &font - fonts.data();
        // add font
        fonts.push_back(make_pair(str, nullptr));
        new RequestXHR(RequestXHR::TypeFont, str, Context::strMng.get(str));
        return fonts.size() - 1;
    }

    bool Application::executeToggleVisible(StringId widgetId) {
        auto len(getWidgetRange(widgetId));
        const char* idSearch(Context::strMng.get(widgetId));
        bool toggled(false);
        for (auto& widget: widgets) {
            if (!memcmp(Context::strMng.get(widget.first), idSearch, len)) {
                widget.second->toggleVisible();
                toggled = true;
            }
        }
        return toggled;
    }

    bool Application::executeQuery(StringId query, StringId widgetId) {
        // execute only in visible widgets
        auto it(widgets.find(widgetId));
        DIAG(
            if (it == widgets.end())
                LOG("internal: cannot find widget %s to send query", Context::strMng.get(widgetId)));
        if (it->second->visible)
            new RequestXHR(RequestXHR::TypeTemplate, widgetId, Context::strMng.get(query));
        return true;
    }

    int Application::getWidgetRange(StringId widgetId) const {
        const char* id(Context::strMng.get(widgetId));
        int len(strlen(id));
        if (len && id[len - 1] == '*') return len - 1;
        return len + 1;
    }

    DIAG(
        void Application::dump() const {
            if (root) {
                LOG("Application tree");
                root->dump();
            }
            LOG("Registered widgets:");
            for (const auto& widget: widgets)
                widget.second->dump(-1, true);
            LOG("Action tables");
            for (const auto& actionTable: actionTables) {
                LOG("  %ld", &actionTable - actionTables.data());
                for (auto action: actionTable.actions)
                    LOG("    %d", action);
            }
        });

}
