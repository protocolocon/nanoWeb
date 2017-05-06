/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include <vector>

namespace render {

    class Atlas {
    public:
        Atlas();
        ~Atlas();
        bool init();
        void finish();

        // sprite storage
        struct Sprite {
            uint16_t x[2], y[2];
            Sprite() { }
            Sprite(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1): x{x0, x1}, y{y0, y1} { }
            inline bool operator==(const Sprite& s) const { return x[0] == s.x[0] && x[1] == s.x[1] && y[0] == s.y[0] && y[1] == s.y[1]; }
        };

        inline const Sprite& get(int i) const { return sprites.at(i); }

        // add bitmap (just alpha)
        void addBitmap(int iSprite, const uint8_t* bitmap);

        // add space for new sprite, returning id of sprite or -1
        int add(int width, int height);

        // remove sprite
        void remove(int sprite);

        // debug
        void dump() const;
        bool check() const;

    private:
        std::vector<Sprite> sprites;
        std::vector<uint32_t> spriteHoles;

        // fast access to free space
        struct IndexCell {
            uint32_t sprite;
            uint32_t next;
        };
        std::vector<uint32_t> cells;        // 2D array of sprites per cell
        std::vector<IndexCell> linkedCells; // first one not used
        uint32_t cellFree;                  // index of the first free cell

        // texture id in the GPU
        GLuint glTextureId;

        int tryAdd(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
        int getFreeSprite();
        int getFreeCell();
        void returnSprite(int sprite);
        void returnCell(int cell);
    };

}
