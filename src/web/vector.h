/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <limits>
#include <cstdlib>
#include <cstdint>
#include <algorithm>

namespace webui {

    template <typename C>
    class Vector {
    public:
        union {
            C v[2];
            struct {
                C x;
                C y;
            };
        };

        inline Vector(): x(0), y(0) {
        }

        inline Vector(C x, C y): x(x), y(y) {
        }

        inline Vector(const Vector& c) {
            assign(c);
        }

        inline void assign(C cx, C cy) {
            x = cx;
            y = cy;
        }

        inline void assign(const Vector& c) {
            x = c.x;
            y = c.y;
        }

        inline Vector& operator+=(const Vector& s) {
            x += s.x;
            y += s.y;
            return *this;
        }

        inline Vector& operator-=(const Vector& s) {
            x -= s.x;
            y -= s.y;
            return *this;
        }

        inline Vector& operator*=(C s) {
            x *= s;
            y *= s;
            return *this;
        }

        inline Vector& operator/=(C s) {
            x /= s;
            y /= s;
            return *this;
        }

        inline Vector operator+(const Vector& s) const {
            return Vector(x + s.x, y + s.y);
        }

        inline Vector operator-(const Vector& s) const {
            return Vector(x - s.x, y - s.y);
        }

        inline Vector operator*(C s) const {
            return Vector(x * s, y * s);
        }

        inline Vector operator/(C s) const {
            return Vector(x / s, y / s);
        }

        inline Vector operator>>(int r) const {
            return Vector(x >> r, y >> r);
        }

        inline Vector operator-() const {
            return Vector(-x, -y);
        }

        inline bool operator==(const Vector& o) const {
            return close(o);
        }

        inline bool operator!=(const Vector& o) const {
            return x != o.x || y != o.y;
        }

        inline bool operator>=(const Vector& o) const {
            return x >= o.x && y >= o.y;
        }

        inline void operator=(const Vector& o) {
            x = o.x;
            y = o.y;
        }

        inline bool operator<(const Vector& o) const {
            return x < o.x && y < o.y;
        }

        inline C operator[](int i) const {
            return v[i];
        }

        inline C& operator[](int i) {
            return v[i];
        }

        inline Vector min(const Vector& v) const {
            return Vector(std::min(x, v.x), std::min(y, v.y));
        }

        static inline bool close(C a, C b, C epsilon = std::numeric_limits<C>::epsilon()) {
            return a-b <= epsilon && b-a <= epsilon;
        }

        inline bool close(const Vector& o, C eps = std::numeric_limits<C>::epsilon()) const {
            return close(x, o.x, eps) && close(y, o.y, eps);
        }

        inline C manhatan(const Vector& v) const {
            return this->abs(x - v.x) + this->abs(y - v.y);
        }

        inline static C abs(C a) {
            return a < 0 ? -a : a;
        }

    };

    using V2s = Vector<short>;
    using V2i = Vector<int>;
    using V2f = Vector<float>;
    using V2d = Vector<double>;
    using V2us = Vector<uint16_t>;

    template <typename C>
    struct Box {
        Box() { }
        Box(const Box& b): x0(b.x0), y0(b.y0), x1(b.x1), y1(b.y1) { }
        Box(C x, C y, C w, C h): pos(x, y), size(w, h) { }
        Box& operator=(const Box& b) { x0 = b.x0; y0 = b.y0; x1 = b.x1; y1 = b.y1; return *this; }
        void intersect(Box b) {
            if (b.pos.x > pos.x) { size.x -= b.pos.x - pos.x; pos.x = b.pos.x; }
            if (b.pos.y > pos.y) { size.y -= b.pos.y - pos.y; pos.y = b.pos.y; }
            if (b.pos.x + b.size.x < pos.x + size.x) size.x = b.pos.x + b.size.x - pos.x;
            if (b.pos.y + b.size.y < pos.y + size.y) size.y = b.pos.y + b.size.y - pos.y;
        }
        inline bool operator==(const Box& b) const { return x0 == b.x0 && y0 == b.y0 && x1 == b.x1 && y1 == b.y1; }
        inline C& operator[](int i) { return v[i]; }
        inline C operator[](int i) const { return v[i]; }
        inline C width() const { return x1 - x0; }
        inline C height() const { return y1 - y0; }
        union {
            struct {
                Vector<C> pos;
                Vector<C> size;
            };
            struct {
                C x0, y0, x1, y1;
            };
            C v[4];
        };
    };

    using Box4f = Box<float>;
    using Box4us = Box<uint16_t>;

}
