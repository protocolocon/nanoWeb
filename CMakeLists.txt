cmake_minimum_required (VERSION 2.8.12)

# this is the CMake build script for the browser or desktop code
option(NANO_BROWSER "Build for the Web (locally if deactivated)" ON)

# the less file formats activated the better (browser code will be smaller)
option(CMAKE_BUILD_TYPE "Build type" "Release")
option(NANO_JPEG "Add support for jpeg images" ON)
option(NANO_PNG "Add support for png images" ON)
option(NANO_GIF "Add support for gif images" ON)
option(NANO_BMP "Add support for bmp images" OFF)
option(NANO_PSD "Add support for psd images" OFF)
option(NANO_TGA "Add support for tga images" OFF)
option(NANO_HDR "Add support for hdr images" OFF)
option(NANO_PIC "Add support for pic images" OFF)
option(NANO_PNM "Add support for pnm (ppm/pgm) images" OFF)
option(NANO_IOSTREAM "Debugging strings support" OFF) # this saves like 1.5 MB of javascript in debug

if(NANO_BROWSER)
  # NOTE: you need emscripten compiler to build this project in the web
  find_program(EMSCRIPTEN_COMPILER emcc)
  if(NOT EXISTS "${EMSCRIPTEN_COMPILER}")
    message(FATAL_ERROR "Cannot find Emscripten compiler.\n"
      "Did you forget to set the environment?\n"
      "Try following these instructions:\n"
      "https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html")
  endif()
  #set(CMAKE_TOOLCHAIN_FILE "$ENV{EMSCRIPTEN}/cmake/Modules/Platform/Emscripten.cmake")
  set(CMAKE_C_COMPILER emcc)
  set(CMAKE_CXX_COMPILER em++)
endif()

project("nanoWeb")

enable_language(C)
enable_language(CXX)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

# check dependencies
if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/submodules/nanovg/src")
  message(FATAL_ERROR "It seems dependencies are missing.\n"
    "Try 'git submodule update --init --recursive'")
endif()

SET(FLAGS "")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  SET(FLAGS "${FLAGS} -O0 -g")
else()
  SET(CMAKE_BUILD_TYPE "Release")
  SET(FLAGS "${FLAGS} -O3")
endif()
if(NOT NANO_JPEG)
  SET(FLAGS "${FLAGS} -DSTBI_NO_JPEG")
endif()
if(NOT NANO_PNG)
  SET(FLAGS "${FLAGS} -DSTBI_NO_PNG")
endif()
if(NOT NANO_BMP)
  SET(FLAGS "${FLAGS} -DSTBI_NO_BMP")
endif()
if(NOT NANO_PSD)
  SET(FLAGS "${FLAGS} -DSTBI_NO_PSD")
endif()
if(NOT NANO_TGA)
  SET(FLAGS "${FLAGS} -DSTBI_NO_TGA")
endif()
if(NOT NANO_GIF)
  SET(FLAGS "${FLAGS} -DSTBI_NO_GIF")
endif()
if(NOT NANO_HDR)
  SET(FLAGS "${FLAGS} -DSTBI_NO_HDR")
endif()
if(NOT NANO_PIC)
  SET(FLAGS "${FLAGS} -DSTBI_NO_PIC")
endif()
if(NOT NANO_PNM)
  SET(FLAGS "${FLAGS} -DSTBI_NO_PNM")
endif()
SET(FLAGS "${FLAGS} -DSTBI_NO_SIMD")

if(NANO_BROWSER)
  SET(FLAGS "${FLAGS} -DSTBI_NO_STDIO")
  set(FLAGS "${FLAGS} -s EXPORTED_FUNCTIONS=\"['_javascriptCanvasResize','_main']\" -s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1 -s USE_GLFW=3")
  if(NANO_IOSTREAM)
    SET(FLAGS "${FLAGS} -DNANO_IOSTREAM")
  endif()
else()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lglfw -lGL")
endif()
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")

message("BUILD_TYPE:       ${CMAKE_BUILD_TYPE}")
message("CXX_FLAGS:        ${CMAKE_CXX_FLAGS}")
message("EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

include_directories(                                                                                                                                           |
  ${PROJECT_SOURCE_DIR}/submodules/nanovg/src)

add_executable(nanoWeb.js
  src/web/main.cc
  src/web/render.cc src/web/render.h
  src/web/definition_common.h
  submodules/nanovg/src/nanovg.c)
