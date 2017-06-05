if(NANO_BROWSER)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_FILESYSTEM=0 --memory-init-file 0") # for SOCKFS, no independent .mem
else()
  include_directories(
    ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src/protocol)

if(NANO_USE_FREETYPE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFT2_BUILD_LIBRARY")
  include_directories(${PROJECT_SOURCE_DIR}/submodules/freetype2/freetype2/include)
  set(FTDIR "${PROJECT_SOURCE_DIR}/submodules/freetype2/freetype2/src")
  add_library(freetype
    ${FTDIR}/base/ftbase.c
    ${FTDIR}/base/ftinit.c
    ${FTDIR}/base/ftsystem.c
    ${FTDIR}/sfnt/sfnt.c
    ${FTDIR}/smooth/smooth.c
    ${FTDIR}/truetype/truetype.c
  )

endif()
if(NANO_USE_STB_TT)
  include_directories(${PROJECT_SOURCE_DIR}/submodules/nanovg/src)
endif()

# render
add_library(client_lib
  font.cc
  atlas.cc
  vertex.cc
  render.cc
  font_ll.cc
  context.cc
  client_app.cc
  communication.cc
  compatibility.cc)

if(NOT NANO_BROWSER)
  target_link_libraries(client_lib uwebsockets)
endif()

if(NANO_USE_FREETYPE)
  target_link_libraries(client_lib freetype)
endif()
