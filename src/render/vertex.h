/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "compatibility.h"
#include <vector>

namespace render {

    struct RGBA {
        union {
            uint32_t c;
            struct {
                uint8_t a, b, g, r;
            };
        };
        RGBA(): c(0) { }
        RGBA(uint32_t c): c(c) { }
    };


    struct Vertex {
        GLfloat pos[2];
        GLushort tex[2];
        RGBA color;
    };


    class VertexBuffer {
    public:
        VertexBuffer();
        ~VertexBuffer();
        bool init();
        void finish();

        void clear();
        Vertex* addTriangle(); // 3 vertices

        bool render(int glMode);

    private:
        GLuint vbo;    // vertex buffer object
        std::vector<Vertex> vertices;
    };

}
