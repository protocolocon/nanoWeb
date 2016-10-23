cmake_minimum_required (VERSION 2.8.12)

# this is the CMake build script for the browser or desktop code
option(NANO_BROWSER "Build for the Web (for the desktop if OFF)" ON)

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
option(NANO_STDIO "Debugging strings support" OFF)
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
  # cannot make this work
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
if(NOT NANO_BROWSER)
  find_package(OpenGL)
  if (NOT OPENGL_FOUND)
    message(FATAL_ERROR "Cannot find libGL in the system")
  endif()
  find_library(LIB_GLFW NAMES glfw)
  if (NOT EXISTS "${LIB_GLFW}")
    message(FATAL_ERROR "Cannot find libglfw in the system")
  endif()
  find_package(CURL)
  if (NOT CURL_FOUND)
    message(FATAL_ERROR "Cannot find libcurl in the system")
  endif()
endif()

set(FLAGS "")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(FLAGS "${FLAGS} -O0 -g")
else()
  set(CMAKE_BUILD_TYPE "Release")
  set(FLAGS "${FLAGS} -O3")
  if(NANO_BROWSER)
    set(FLAGS "${FLAGS} --llvm-lto 1")
  endif()
endif()
if(NOT NANO_JPEG)
  set(FLAGS "${FLAGS} -DSTBI_NO_JPEG")
endif()
if(NOT NANO_PNG)
  set(FLAGS "${FLAGS} -DSTBI_NO_PNG")
endif()
if(NOT NANO_BMP)
  set(FLAGS "${FLAGS} -DSTBI_NO_BMP")
endif()
if(NOT NANO_PSD)
  set(FLAGS "${FLAGS} -DSTBI_NO_PSD")
endif()
if(NOT NANO_TGA)
  set(FLAGS "${FLAGS} -DSTBI_NO_TGA")
endif()
if(NOT NANO_GIF)
  set(FLAGS "${FLAGS} -DSTBI_NO_GIF")
endif()
if(NOT NANO_HDR)
  set(FLAGS "${FLAGS} -DSTBI_NO_HDR")
endif()
if(NOT NANO_PIC)
  set(FLAGS "${FLAGS} -DSTBI_NO_PIC")
endif()
if(NOT NANO_PNM)
  set(FLAGS "${FLAGS} -DSTBI_NO_PNM")
endif()
set(FLAGS "${FLAGS} -DSTBI_NO_SIMD")

if(NANO_BROWSER)
  set(FLAGS "${FLAGS} -DSTBI_NO_STDIO")
  set(FLAGS "${FLAGS} -s EXPORTED_FUNCTIONS=\"['_javascriptCanvasResize','_main']\"")
  set(FLAGS "${FLAGS} -s ELIMINATE_DUPLICATE_FUNCTIONS=1 -s NO_FILESYSTEM=1 -s USE_GLFW=3")
  if(NANO_IOSTREAM)
    set(FLAGS "${FLAGS} -DNANO_IOSTREAM")
  endif()
  if(NANO_STDIO)
    set(FLAGS "${FLAGS} -DNANO_STDIO")
  else()
    set(FLAGS "${FLAGS} -DSTBI_NO_STDIO")
    set(FLAGS "${FLAGS} -DNVG_NO_STDIO")
  endif()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EMTERPRETIFY=0")
  set(TARGET_NAME "nanoWeb.js")
else()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lglfw -lGL -lcurl")
  set(TARGET_NAME "nanoWeb")
endif()
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS} -std=c++14 -fno-exceptions -fno-rtti")

message("BUILD_TYPE:       ${CMAKE_BUILD_TYPE}")
message("CXX_FLAGS:        ${CMAKE_CXX_FLAGS}")
message("EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

include_directories(                                                                                                                                           |
  ${PROJECT_SOURCE_DIR}/submodules/json
  ${PROJECT_SOURCE_DIR}/submodules/nanovg/src)

add_executable(${TARGET_NAME}
  src/web/main.cc
  src/web/render.cc
  src/web/application.cc
  src/web/compatibility.cc
  submodules/json/json.c
  submodules/nanovg/src/nanovg.c)
