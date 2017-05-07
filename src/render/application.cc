/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "application.h"
#include "font.h"
#include "context.h"
#include <cassert>
#include <cstring>
#include <cstdlib>

using namespace prot;

namespace render {

    Application::Application(): delayProgress(0) {
    }

    void Application::refresh() {
        if (!serverRefresh()) {
            // server context is lost
            LOG("server communication restarted");
            reset();
        }

        // try to execute delayed commands (when not in the middle of a transfer)
        if (!transferProgress) {
            while (delayProgress < int(delay.size())) {
                assert(delay.size() - delayProgress > sizeof(ProtoBase));
                expectTransfer(sizeof(ProtoBase));
                memcpy(transferBuffer.data(), &delay[delayProgress], sizeof(ProtoBase));
                ProtoBase* base(reinterpret_cast<ProtoBase*>(transferBuffer.data()));
                continueTransfer(sizeof(ProtoBase) + base->size);
                memcpy(transferBuffer.data() + sizeof(ProtoBase), &delay[delayProgress] + sizeof(ProtoBase), base->size);
                if (!execute(true)) break;
                delayProgress += transferBuffer.size();
            }
            if (delayProgress && delayProgress == int(delay.size())) {
                delayProgress = 0;
                delay.clear();
            }
            reset();
        }

        // read cycle from network
        int read;
        assert(transferProgress < transferBuffer.size());
        while ((read = serverRead(&transferBuffer[transferProgress], transferBuffer.size() - transferProgress))) {
            transferProgress += read;
            if (transferProgress == transferBuffer.size()) {
                // completed transfer
                ProtoBase* base(reinterpret_cast<ProtoBase*>(transferBuffer.data()));
                if (transferProgress == sizeof(ProtoBase)) continueTransfer(sizeof(ProtoBase) + base->size);
                if (transferProgress == transferBuffer.size()) {
                    execute(false);
                    reset();
                }
            }
        }
    }

    bool Application::execute(bool delayed) {
        ProtoBase* base(reinterpret_cast<ProtoBase*>(transferBuffer.data()));
        switch (base->com) {
        case Command::Text: return doDelayIfFailed(delayed, commandText());
        case Command::Font: commandFont(); return true;
        }
        return false;
    }

    void Application::doDelay() {
        // last command needs to be delayed
        assert(transferBuffer.size() > sizeof(ProtoBase));
        ProtoBase* base(reinterpret_cast<ProtoBase*>(transferBuffer.data()));
        int size(delay.size());
        delay.resize(delay.size() + sizeof(ProtoBase) + base->size);
        memcpy(&delay[size], transferBuffer.data(), transferBuffer.size());
        LOG("delayed command: %d -> %d bytes", int(base->com), base->size);
    }

    bool Application::doDelayIfFailed(bool delayed, bool ok) {
        if (!delayed && !ok) doDelay();
        return ok;
    }

    bool Application::commandText() {
        Text* text(reinterpret_cast<Text*>(transferBuffer.data()));
        if (text->font >= int(fonts.size()) || !fonts[text->font].initialized()) {
            missingFont(text->font);
            return false;
        } else {
            LOG("text %d %d %f %f %.*s", text->font, text->fontSize, text->x, text->y, int(transferBuffer.size() - sizeof(Text)), text->str);
            fonts[text->font].text(text->str, text->str + transferBuffer.size() - sizeof(Text), text->fontSize, atlas, vertexBuffer);
            return true;
        }
    }

    void Application::commandFont() {
        FontResource* font(reinterpret_cast<FontResource*>(transferBuffer.data()));
        int size(int(transferBuffer.size() - sizeof(FontResource)));
        LOG("received font %d: %d bytes", font->font, int(transferBuffer.size() - sizeof(FontResource)));
        // duplicate data
        uint8_t* fft((uint8_t*)malloc(size));
        memcpy(fft, font->data, size);
        // initialize font
        assert(font->font < int(fonts.size()) && fonts[font->font].initializationInProgress());
        if (!fonts[font->font].init(fft, int(transferBuffer.size() - sizeof(FontResource))))
            LOG("error: cannot initialize font: %d", font->font);
        assert(font->font < int(fonts.size()) && fonts[font->font].initialized());
        //fonts[font->font].populate(32, atlas);
    }

    void Application::missingFont(int fontIdx) {
        if (fontIdx >= int(fonts.size())) fonts.resize(fontIdx + 1);
        if (!fonts[fontIdx].initializationInProgress()) {
            LOG("resource font %d missing", fontIdx);
            // ask for resource
            FontResource font(fontIdx, 0);
            if (serverWrite((uint8_t*)&font, sizeof(font)) != sizeof(font))
                LOG("cannot send");
            else
                fonts[fontIdx].markInitInProgress();
        }
    }

}
