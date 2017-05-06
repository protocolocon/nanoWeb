/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "vertex.h"
#include "render.h"

namespace render {

    VertexBuffer::VertexBuffer(): vbo(0) {
    }

    VertexBuffer::~VertexBuffer() {
        finish();
    }

    bool VertexBuffer::init() {
        finish();

        // prepare VAO
        glEnableVertexAttribArray(LOC_VERTEX);
        glEnableVertexAttribArray(LOC_TEXTURE);
        glEnableVertexAttribArray(LOC_COLOR);

        // init VBO's
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(LOC_VERTEX,  2, GL_FLOAT,          GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, pos)));
        glVertexAttribPointer(LOC_TEXTURE, 2, GL_UNSIGNED_SHORT, GL_TRUE,  sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tex)));
        glVertexAttribPointer(LOC_COLOR,   4, GL_UNSIGNED_BYTE,  GL_TRUE,  sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        return Render::checkError();
    }

    void VertexBuffer::finish() {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    void VertexBuffer::clear() {
        vertices.clear();
    }

    Vertex* VertexBuffer::addTriangle() {
        vertices.resize(vertices.size() + 3);
        return &vertices.back() - 2;
    }

    bool VertexBuffer::render(int glMode) {
        // update complete buffer data: TODO, probably not
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // render
        glDrawArrays(glMode, 0, vertices.size());
        return Render::checkError();
    }

}
