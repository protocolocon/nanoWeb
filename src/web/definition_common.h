/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#ifndef GL_GLEXT_PROTOTYPES
#  define GL_GLEXT_PROTOTYPES
#endif

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#  define EMS(x, ...)          x, ##__VA_ARGS__
#  define DSK(x, ...)
#  ifdef NANO_IOSTREAM
#    include <iostream>
#    define IOS(x, ...)        x, ##__VA_ARGS__
#  else
#    define IOS(x, ...)
#  endif
#else
#  include <thread>
#  include <iostream>
#  define IOS(x, ...)        x, ##__VA_ARGS__
#  ifndef GLFW_INCLUDE_GLCOREARB
//#    define GLFW_INCLUDE_GLCOREARB
#  endif
#  define EMS(x, ...)
#  define DSK(x, ...)          x, ##__VA_ARGS__
#endif
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>
