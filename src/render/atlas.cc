/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "atlas.h"
#include "render.h"
#include "vertex.h"
#include <cassert>
#include <cstring>

using namespace std;
using namespace webui;

namespace {

    const uint32_t SizeRot(10);
    const uint32_t Size(1 << SizeRot);
    const uint32_t CellRot(4);
    const uint32_t NCell(1 << (SizeRot - CellRot));

}

namespace render {

    Atlas::Atlas(): glTextureId(0) {
        sprites.reserve(2048);
        cells.assign(NCell * NCell, 0);

        uint32_t nLinked(8192);
        linkedCells.resize(nLinked);
        for (uint32_t i = 0; i < nLinked; i++) linkedCells[i].next = i + 1;
        linkedCells[nLinked - 1].next = 0;
        cellFree = 1;
    }

    Atlas::~Atlas() {
        finish();
    }

    bool Atlas::init() {
        // reset atlas (required for webgl)
        RGBA* pixmap((RGBA*)malloc(Size * Size * sizeof(RGBA)));
        memset(pixmap, 0, Size * Size * sizeof(RGBA));
        glGenTextures(1, &glTextureId);
        glBindTexture(GL_TEXTURE_2D, glTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Size, Size, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixmap);
        free(pixmap);

#if 1
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#else
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        return Render::checkError();
    }

    void Atlas::finish() {
        glDeleteTextures(1, &glTextureId); // also removes the binding
        glTextureId = 0;
    }

    void Atlas::addBitmap(int iSprite, const uint8_t* bitmap) {
        const auto& sprite(sprites[iSprite]);
        auto w(sprite.box.width());
        auto h(sprite.box.height());
        auto s(w * h);
        RGBA* pixmap((RGBA*)malloc(s * sizeof(RGBA)));
        for (int i = 0; i < s; i++) pixmap[i].c = 0xffffff | uint32_t(bitmap[i]) << 24;
        
        glBindTexture(GL_TEXTURE_2D, glTextureId);
        glTexSubImage2D(GL_TEXTURE_2D, 0, sprite.box.x0, sprite.box.y0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixmap);

        // test
#if 0
        for (int j=0; j < h; ++j) { for (int i=0; i < w; ++i) putchar(" .:ioVM@"[bitmap[j*w+i]>>5]); putchar('\n'); } putchar('\n');
        for (int j=0; j < h; ++j) { for (int i=0; i < w; ++i) printf("%08x ", pixmap[j*w+i].c); putchar('\n'); } putchar('\n');
#endif

        free(pixmap);
    }

    int Atlas::add(int width, int height) {
        int index;
        if ((index = tryAdd(0, 0, width, height)) >= 0) return index;
        for (auto& s: sprites) if ((index = tryAdd(s.box.x1 + 1, s.box.y0, width, height)) >= 0) return index;
        for (auto& s: sprites) if ((index = tryAdd(s.box.x0, s.box.y1 + 1, width, height)) >= 0) return index;
        return -1;
    }

    void Atlas::remove(int iSprite) {
        assert(iSprite >= 0 && iSprite < int(sprites.size()));
        auto& sprite(sprites[iSprite]);
        // remove from cells
        for (uint32_t i = sprite.box.y0 >> CellRot; i <= uint32_t(sprite.box.y1 >> CellRot); i++) {
            auto* row(&cells[i * NCell]);
            for (uint32_t j = sprite.box.x0 >> CellRot; j <= uint32_t(sprite.box.x1 >> CellRot); j++) {
                assert(row[j]);
                uint32_t* prev(&row[j]);
                auto* cell(&linkedCells[row[j]]);
                while (true) {
                    if (int(cell->sprite) == iSprite) {
                        int tmp(cell->next);
                        returnCell(*prev);
                        *prev = tmp;
                        break;
                    }
                    if (!cell->next) abort(); // not possible
                    prev = &cell->next;
                    cell = &linkedCells[cell->next];
                }
            }
        }
        // remove sprite
        returnSprite(iSprite);
    }

    int Atlas::tryAdd(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
        uint32_t xx(x + w + 1/*border*/);
        uint32_t yy(y + h + 1/*border*/);
        if (xx > Size || yy > Size) return -1;
        // check cells
        for (uint32_t i = y >> CellRot; i <= (yy-1) >> CellRot; i++) {
            auto* row(&cells[i * NCell]);
            for (uint32_t j = x >> CellRot; j <= (xx-1) >> CellRot; j++) {
                if (row[j]) {
                    auto* cell(&linkedCells[row[j]]);
                    while (true) {
                        auto& sprite(sprites[cell->sprite]);
                        if (sprite.box.x0 < xx && sprite.box.x1 >= x &&
                            sprite.box.y0 < yy && sprite.box.y1 >= y) return -1;
                        if (!cell->next) break;
                        cell = &linkedCells[cell->next];
                    }
                }
            }
        }
        // space found
        int iSprite(getFreeSprite());
        auto& sprite(sprites[iSprite]);
        sprite.box.x0 = x;
        sprite.box.y0 = y;
        sprite.box.x1 = xx - 1;
        sprite.box.y1 = yy - 1;

        // ok, so mark blocks
        for (uint32_t i = y >> CellRot; i <= (yy-1) >> CellRot; i++) {
            auto* row(&cells[i * NCell]);
            for (uint32_t j = x >> CellRot; j <= (xx-1) >> CellRot; j++) {
                auto iCell(getFreeCell());
                auto& cell(linkedCells[iCell]);
                cell.next = row[j];
                row[j] = iCell;
                cell.sprite = iSprite;
            }
        }
        return iSprite;
    }

    int Atlas::getFreeSprite() {
        int i;
        if (spriteHoles.empty()) {
            i = sprites.size();
            sprites.resize(sprites.size() + 1);
        } else {
            i = spriteHoles.back();
            spriteHoles.pop_back();
        }
        return i;
    }

    int Atlas::getFreeCell() {
        if (!cellFree) {
            // get more space
            int i(linkedCells.size());
            linkedCells.resize(i * 2);
            for (int j = i; j < i*2; j++)
                linkedCells[j].next = j + 1;
            linkedCells[i*2 - 1].next = 0;
            cellFree = i;
        }
        assert(cellFree);
        int i(cellFree);
        cellFree = linkedCells[cellFree].next;
        return i;
    }

    void Atlas::returnSprite(int iSprite) {
        spriteHoles.push_back(iSprite);
    }

    void Atlas::returnCell(int iCell) {
        assert(iCell >= 0 && iCell < int(linkedCells.size()));
        linkedCells[iCell].next = cellFree;
        cellFree = iCell;
    }

    void Atlas::dump() const {
        LOG("Sprites:");
        for (auto& sprite: sprites)
            LOG("  %5d:  %4d %4d  : %4d %4d", int(&sprite - sprites.data()), sprite.box.x0, sprite.box.y0, sprite.box.x1, sprite.box.y1);
        LOG("Sprite holes:");
        for (auto& hole: spriteHoles)
            LOG("  %4d", hole);
        LOG("Cells:");
        for (uint32_t i = 0; i < NCell; i++) {
            char buffer[8192];
            int nBuffer(0);
            for (uint32_t j = 0; j < NCell; j++)
                nBuffer += snprintf(buffer + nBuffer, sizeof(buffer) - nBuffer, "%4d", cells[i * NCell + j]);
            LOG("  %s", buffer);
        }
        LOG("Free cell: %d", cellFree);
        LOG("Linked cells:");
        for (auto& link: linkedCells)
            LOG("  %5d: %5d %5d", int(&link - linkedCells.data()), link.sprite, link.next);
    }

    bool Atlas::check() const {
        bool ok(true);

        // check no sprite and linked cells leak
        int nUsedSprite(0), nUsedLink(0);
        vector<bool> usedSprite(sprites.size(), false);
        vector<bool> usedLink(linkedCells.size(), false);
        for (uint32_t i = 0; i < NCell * NCell; i++) {
            if (cells[i]) {
                if (usedLink[cells[i]]) { LOG("error: reused linked list"); ok = false; }
                usedLink[cells[i]] = true;
                nUsedLink++;
                auto* cell(&linkedCells[cells[i]]);
                while (true) {
                    if (!usedSprite[cell->sprite]) nUsedSprite++;
                    usedSprite[cell->sprite] = true;
                    if (!cell->next) break;
                    if (usedLink[cell->next]) { LOG("error: reused linked list"); ok = false; }
                    usedLink[cell->next] = true;
                    nUsedLink++;
                    cell = &linkedCells[cell->next];
                }
            }
        }
        if (nUsedSprite + spriteHoles.size() != sprites.size()) {
            LOG("error: sprite leak");
            ok = false;
        }
        int nFreeLink(0);
        if (cellFree) {
            auto* link(&linkedCells[cellFree]);
            while (true) {
                nFreeLink++;
                if (!link->next) break;
                link = &linkedCells[link->next];
            }
        }
        if (nUsedLink + nFreeLink != int(linkedCells.size()) - 1) {
            LOG("error: link cell leak");
            ok = false;
        }
        return ok;
    }

    Box4us Atlas::Sprite::tex() const {
        Box4us tex;
        tex[0] = box[0] << (16 - SizeRot);
        tex[1] = box[1] << (16 - SizeRot);
        tex[2] = box[2] << (16 - SizeRot);
        tex[3] = box[3] << (16 - SizeRot);
        return tex;
    }

}
