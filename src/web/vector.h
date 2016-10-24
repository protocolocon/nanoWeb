/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <limits>

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

        inline Vector operator-() const {
            return Vector(-x, -y);
        }

        inline bool operator==(const Vector& o) const {
            return close(o);
        }

        inline bool operator!=(const Vector& o) const {
            return !operator==(o);
        }

        inline C operator[](int i) const {
            return v[i];
        }

        inline C& operator[](int i) {
            return v[i];
        }

        static inline bool close(C a, C b, C epsilon = std::numeric_limits<C>::epsilon()) {
            return a-b < epsilon && b-a < epsilon;
        }

        inline bool close(const Vector& o, C eps = std::numeric_limits<C>::epsilon()) const {
            return close(x, o.x, eps) && close(y, o.y, eps);
        }

    };

    using V2i = Vector<int>;
    using V2f = Vector<float>;
    using V2d = Vector<double>;

}
