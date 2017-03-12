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
#include <queue>
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

    priority_queue<WidgetTimer*, vector<WidgetTimer*>, WidgetTimerSorter> timers;

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
        if (root && ((Input::refresh() | refreshTimers()) || !layoutStable)) {
            layoutStable = root->layout(Box4f(0.f, 0.f, float(Context::render.getWidth()), float(Context::render.getHeight())));
            ctx.forceRender();
        }
    }

    bool Application::refreshTimers() {
        bool dev(false);
        auto now(ctx.getTimeMs());
        while (!timers.empty()) {
            auto* timer(timers.top());
            if (timer->nextExecutionMs  <= now) {
                // execute
                DIAG(LOG("executing timer: %s", Context::strMng.get(timer->getId())));
                dev |= timer->refreshTimer();
                // take out from timers
                timers.pop();
                if (timer->repeat) {
                    // add it again
                    timer->nextExecutionMs += timer->delay;
                    timers.push(timer);
                }
            } else
                break;
        }
        return dev;
    }

    void Application::triggerTimers() {
        auto t(timers);
        while (!t.empty()) {
            t.top()->refreshTimer();
            t.pop();
        }
    }

    bool Application::update() {
        assert(root);
        Context::cursor = Cursor::Default;
        auto dev(root->update());
        setCursor(Context::cursor);
        return dev;
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
            if (Input::hoverWidget) {
                const auto& actionTable(getActionTable(Input::hoverWidget->actions));
                assert(actionTable.onHover);
                Context::actions.execute(actionTable.onHover, Input::hoverWidget);
            }
            Context::render.endFrame();
        }
    }

    bool Application::onLoad(RequestXHR* xhr) {
        DIAG(if (!xhr->getData() || !xhr->getNData()) { LOG("empty xhr response"); return false; });
        bool dev(true);
        switch (Identifier(xhr->getId().getId())) {
        case Identifier::Application:
            // main application definition
            assert(!root);
            if (!tree.parse(xhr->getData(), xhr->getNData()) || !(root = initializeConstruct()) || !checkActions()) {
                DIAG(
                    xhr->makeCString();
                    LOG("cannot parse: %s", xhr->getData());
                    clear());
                dev = false;
            } else
                resize(Context::render.getWidth(), Context::render.getHeight());

            //DIAG(tree.dumpTree(); dump());
            tree.clear();
            ctx.forceRender();
            break;
        case Identifier::font: {
            // check that font id matches in font list
            bool ok(false);
            StringId id(xhr->getReq());
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
        default: {
            // pass directly to the widget
            auto it(widgets.find(xhr->getId()));
            if (it == widgets.end()) {
                DIAG(LOG("internal: cannot find template %s", Context::strMng.get(xhr->getId())));
                dev = false;
            } else
                if (!tpl.parse(xhr->getData(), xhr->getNData()) || tpl.empty() || tpl[0].type() != MLParser::EntryType::List) {
                    DIAG(LOG("cannot parse template info or is not a complete list"));
                    dev = false;
                } else {
                    //DIAG(LOG("adding data to widget: %s", Context::strMng.get(xhr->getId())); tpl.dumpTree());
                    dev = it->second->setData();
                }
            break;
        }
        }
        delete xhr;
        return dev;
    }

    bool Application::updateTemplate(WidgetTemplate* tplWidget) {
        bool dev(true);
        iTpl = 1;
        fTpl = tpl[0].next;

        tree.swap(tplWidget->getParser());
        //DIAG(LOG("template with description:"); tree.dumpTree());
        Construct cons(tplWidget, 0, tree.size(), true, false);
        if (!initializeConstructCheckUpdate(cons)) {
            DIAG(LOG("error: update template"));
            dev = false;
        } else {
            //DIAG(dump());
            layoutStable = false;
            ctx.forceRender();
        }
        tree.swap(tplWidget->getParser());
        return dev;
    }

    void Application::onError(RequestXHR* xhr) {
        LOG("lost query: %s", Context::strMng.get(xhr->getId()));
        delete xhr;
    }

    Widget* Application::initializeConstruct() {
        DIAG(tree.dumpTree());
        if (!tree.empty() && tree[0].type() == MLParser::EntryType::Object) {
            auto* widget(createWidget(tree.asId(0), nullptr));
            iTpl = fTpl = -1;
            Construct cons(widget, 1, tree.size(), true, false);
            if (widget && (widget = initializeConstructCheckUpdate(cons)) && registerWidget(widget))
                return widget;
        } DIAG(else LOG("expecting object as root of application"));
        return nullptr;
    }

    Widget* Application::initializeConstructCheckUpdate(Construct& cons) {
        cons.constUpdatedOrig = cons.widget->constUpdated;
        cons.widget->constUpdated = 1;
        for (auto child: cons.widget->getChildren()) child->constUpdated = 0;
        auto widget(initializeConstructRecur(cons));
        if (widget) {
            // remove non-updated children
            auto& children(widget->getChildren());
            for (auto it = children.begin(); it != children.end(); )
                if (!(*it)->constUpdated && !(*it)->constStructural)
                    it = children.erase(it);
                else
                    ++it;
        }
        return widget;
    }

    Widget* Application::initializeConstructRecur(Construct& cons) {
        auto iEntryOrig(cons.iEntry);
        while (cons.iEntry < cons.fEntry) {
            // cases:
            //   1. key : value
            //   2. id { }
            //   3. [ ] <- block
            auto valEntry(cons.iEntry + 1);
            const auto& entryKey(tree[cons.iEntry]);
            if (entryKey.type() == MLParser::EntryType::Id) { // case 1: key : value
                assert(valEntry < cons.fEntry && tree[cons.iEntry].next == valEntry);
                auto key(tree.asId(cons.iEntry));
                if (key == Identifier::InvalidId) {
                    DIAG(LOG("invalid key for widget: %.*s", tree.size(cons.iEntry), entryKey.pos));
                    return nullptr;
                }
                // new widget type definition
                if (key == Identifier::define) {
                    key = Identifier::id;
                    DIAG(if (cons.update) LOG("no definitions allowed inside templates"));
                    DIAG(if (cons.define) LOG("warning: repeated 'define' attribute inside widget"));
                    cons.define = true;
                    cons.widget->parent = nullptr;
                    // create new type
                    auto newTypeId(tree.asIdAdd(valEntry));
                    if (!(cons.widget = createType(cons.widget, newTypeId, iEntryOrig, cons.fEntry))) {
                        DIAG(LOG("cannot create new type: %s", Context::strMng.get(newTypeId)));
                        return nullptr;
                    }
                }
                if (int(key) > int(Identifier::ALast) && int(key) <= int(Identifier::PLast)) {
                    // addition of properties in widget definition
                    DIAG(if (!cons.define && iTpl <= 0) {
                            LOG("defining a property for a widget that is not being defined");
                            tree.error(entryKey.pos, "=>", entryKey.line);
                            return nullptr;
                        });
                } else {
                    auto id(cons.widget->id);
                    if(!Context::actions.evalProperty(tree, valEntry, tree[valEntry].next, key, cons.widget, cons.update, cons.define)) { // eval property
                        DIAG(
                            LOG("warning: unknown attribute %s with value %.*s", Context::strMng.get(key), tree.size(valEntry), tree[valEntry].pos);
                            tree.error(entryKey.pos, "=>", entryKey.line);
                            return nullptr);
                    }
                    if (key == Identifier::id && id.getId() != cons.widget->id.getId()) {
                        // updating id of widget:
                        // - if the id already exists, update that one instead
                        // - if not, and this is an update operation, create a new widget
                        auto origWidget(cons.widget);
                        auto it(widgets.find(cons.widget->id));
                        if (it != widgets.end()) {
                            DIAG(LOG("move from widget %s to %s", Context::strMng.get(id), Context::strMng.get(cons.widget->id)));
                            DIAG(if (cons.widget->parent != it->second->parent) LOG("modifying some wrong widget in the application"));
                            cons.widget = it->second;
                            cons.widget->constUpdated = 1;
                            origWidget->id = id; // restore previous id for original widget
                            origWidget->constUpdated = cons.constUpdatedOrig;
                            cons.update = true;
                        } else if (cons.update) {
                            DIAG(LOG("creating new widget for id: %s", Context::strMng.get(cons.widget->id)));
                            cons.widget = createWidget(cons.widget->type(), cons.widget->parent);
                            cons.widget->id = origWidget->id;
                            cons.widget->constUpdated = 1;
                            origWidget->id = id; // restore previous id for original widget
                            origWidget->constUpdated = cons.constUpdatedOrig;
                            cons.update = false;
                        }
                    }
                }
                cons.iEntry = tree[valEntry].next;

            } else if (entryKey.type() == MLParser::EntryType::Object) { // case 2: object { }
                auto key(tree.asId(cons.iEntry));
                DIAG(if (!isWidget(key)) {
                         LOG("expecting object id");
                         tree.error(entryKey.pos, "=>", entryKey.line);
                    });
                if (cons.recurse) {
                    // child widget
                    bool update(false);
                    Widget* widgetChild;
                    while (cons.iChild < cons.widget->getChildren().size()) {
                        if (cons.widget->getChildren()[cons.iChild]->type() == key) {
                            widgetChild = cons.widget->getChildren()[cons.iChild]; // reuse child from current widget
                            //DIAG(LOG("reusing widget: %s", Context::strMng.get(widgetChild->getId())));
                            update = true;
                            break;
                        }
                        cons.iChild++;
                    }
                    if (!update)
                        widgetChild = createWidget(key, cons.widget/*parent*/);
                    DIAG(if (!widgetChild) {
                            LOG("unknown widget type: %.*s", tree.size(cons.iEntry), entryKey.pos);
                            return nullptr;
                        });
                    bool isTemplate(key == Identifier::Template);
                    if (isTemplate) {
                        auto* tpl(reinterpret_cast<WidgetTemplate*>(widgetChild));
                        // omit definition inside template
                        int omitDefine(valEntry < cons.fEntry && tree.asId(valEntry) == Identifier::define ? 2 : 0);
                        tree.copyTo(tpl->getParser(), valEntry + omitDefine, tree[cons.iEntry].next);
                    }
                    Construct consChild(widgetChild, valEntry, tree[cons.iEntry].next, !isTemplate, update);
                    if (!(widgetChild = initializeConstructCheckUpdate(consChild)) || (!consChild.update && !registerWidget(widgetChild)))
                        return nullptr;
                    if (!consChild.define) {
                        if (!consChild.update) cons.widget->addChild(widgetChild);
                        cons.iChild++;
                    }
                    if (widgetChild->baseType() == Identifier::Timer) {
                        // add to list of timers
                        auto* timer(reinterpret_cast<WidgetTimer*>(widgetChild));
                        timer->nextExecutionMs = ctx.getTimeMs();
                        if (!timer->repeat) timer->nextExecutionMs += timer->delay;
                        timers.push(timer);
                    }
                }
                cons.iEntry = tree[cons.iEntry].next;

            } else if (entryKey.type() == MLParser::EntryType::Block) { // case 3: block
                if (cons.recurse) {
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
                            delete cons.widget;
                            return nullptr;
                        }
                    } else
                        DIAG(LOG("error: template iteration, but no template data"));

                    //LOG("ITERATION: %d %d", iIter, fIter);
                    int iEntry(cons.iEntry);
                    int fEntry(cons.fEntry);
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
                        //LOG("   WIDGET: %d %d", iWidget, fWidget);
                        iTpl = iWidget;
                        fTpl = fWidget;
                        cons.iEntry = valEntry;
                        cons.fEntry = tree[valEntry].next;
                        if (!initializeConstructRecur(cons)) {
                            DIAG(LOG("template loop initialize construct"));
                            return nullptr;
                        }
                        DIAG(if (cons.define) LOG("error: definition inside loop"));
                        DIAG(
                            if (iTpl < fTpl)
                                LOG("trailing template values after widget: %d %d", iTpl, fTpl));
                        // prepare next template list (widget)
                        iIter = fWidget; // next widget
                    }
                    iTpl = fIter;
                    fTpl = fTplSave;
                    cons.iEntry = iEntry;
                    cons.fEntry = fEntry;
                }
                cons.iEntry = tree[cons.iEntry].next;
            }
        }
        return cons.widget;
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
                DIAG(if (entryKey.type() != MLParser::EntryType::Object && entryKey.type() != MLParser::EntryType::Id) {
                        LOG("expected key: value or class {");
                        tree.error(entryKey.pos, "=>", entryKey.line);
                        return nullptr;
                    });
                Property prop;
                prop.all = 0;
                Identifier key = tree.asId(iEntry);
                switch (key) {
                case Identifier::propInt16: prop.type = Type::Int16; prop.size = 2; break;
                case Identifier::propFloat: prop.type = Type::Float; prop.size = 4; break;
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
                if (entryKey.type() == MLParser::EntryType::Object)
                    iEntry = entryKey.next;
                else
                    iEntry = tree[iEntry + 1].next;
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
        unordered_set<int> iActions;
        for (auto& idWidget: widgets) {
            auto& table(actionTables[idWidget.second->actions]);
            for (auto action: table.actions) {
                if (!iActions.count(action)) {
                    iActions.insert(action);
                    dev &= Context::actions.execute<true>(action, idWidget.second);
                }
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
        case Identifier::onHover:        table.onHover        = iAction; break;
        default: DIAG(LOG("unknown action")); return false;
        }
        return true;
    }

    int Application::getFont(StringId str) {
        for (auto& font: fonts)
            if (font.first == str) return &font - fonts.data();
        // add font
        fonts.push_back(make_pair(str, nullptr));
        new RequestXHR(Identifier::font, str);
        return fonts.size() - 1;
    }

    bool Application::executeQuery(StringId query, StringId widgetId) {
        DIAG(if (widgets.find(widgetId) == widgets.end()) {
                LOG("internal: cannot find widget %s to send query", Context::strMng.get(widgetId));
                return false;
            });
        new RequestXHR(widgetId, query);
        return true;
    }

    int Application::getWidgetRange(StringId widgetId) const {
        const char* id(Context::strMng.get(widgetId));
        int len(strlen(id));
        if (len && id[len - 1] == '*') return len - 1;
        return len + 1;
    }

    DIAG(
        void Application::dump(bool detail, bool actions) const {
            if (root) {
                LOG("Application tree");
                root->dump();
                LOG("Definitions");
                for (auto idWidget: widgets) {
                    auto* widget(idWidget.second);
                    if (widget != root && !widget->parent)
                        widget->dump(1);
                }
            }
            if (detail) {
                LOG("Registered widgets:");
                for (const auto& widget: widgets)
                    widget.second->dump(-1, true);
            }
            if (actions) {
                LOG("Action tables");
                for (const auto& actionTable: actionTables) {
                    LOG("  %ld", &actionTable - actionTables.data());
                    for (auto action: actionTable.actions)
                        LOG("    %d", action);
                }
            }
        });

}
