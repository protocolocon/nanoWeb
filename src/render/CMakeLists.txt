if(NANO_BROWSER)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_FILESYSTEM=0 --memory-init-file 0") # for SOCKFS, no independent .mem
else()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -lssl -lcrypto -lz")

  include_directories(
    ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src/protocol)

if(NANO_USE_FREETYPE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFT2_BUILD_LIBRARY")
  include_directories(${PROJECT_SOURCE_DIR}/external/freetype-2.7.1/include)
  set(FTDIR "${PROJECT_SOURCE_DIR}/external/freetype-2.7.1/src")
  add_library(freetype
    #${FTDIR}/autofit/autofit.c
    ${FTDIR}/base/ftbase.c
    #${FTDIR}/base/ftbbox.c
    #${FTDIR}/base/ftbdf.c
    #${FTDIR}/base/ftbitmap.c
    #${FTDIR}/base/ftcid.c
    #${FTDIR}/base/ftfntfmt.c
    #${FTDIR}/base/ftfstype.c
    #${FTDIR}/base/ftgasp.c
    #${FTDIR}/base/ftglyph.c
    #${FTDIR}/base/ftgxval.c
    ${FTDIR}/base/ftinit.c
    #${FTDIR}/base/ftlcdfil.c
    #${FTDIR}/base/ftmm.c
    #${FTDIR}/base/ftotval.c
    #${FTDIR}/base/ftpatent.c
    #${FTDIR}/base/ftpfr.c
    #${FTDIR}/base/ftstroke.c
    #${FTDIR}/base/ftsynth.c
    ${FTDIR}/base/ftsystem.c
    #${FTDIR}/base/fttype1.c
    #${FTDIR}/base/ftwinfnt.c
    #${FTDIR}/bdf/bdf.c
    #${FTDIR}/bzip2/ftbzip2.c
    #${FTDIR}/cache/ftcache.c
    #${FTDIR}/cff/cff.c
    #${FTDIR}/cid/type1cid.c
    #${FTDIR}/gzip/ftgzip.c
    #${FTDIR}/lzw/ftlzw.c
    #${FTDIR}/pcf/pcf.c
    #${FTDIR}/pfr/pfr.c
    #${FTDIR}/psaux/psaux.c
    #${FTDIR}/pshinter/pshinter.c
    #${FTDIR}/psnames/psnames.c
    #${FTDIR}/raster/raster.c
    ${FTDIR}/sfnt/sfnt.c
    ${FTDIR}/smooth/smooth.c
    ${FTDIR}/truetype/truetype.c
    #${FTDIR}/type1/type1.c
    #${FTDIR}/type42/type42.c
    #${FTDIR}/winfonts/winfnt.c
  )

endif()
if(NANO_USE_STB_TT)
  include_directories(${PROJECT_SOURCE_DIR}/submodules/nanovg/src)
endif()

# render
add_library(render_lib
  font.cc
  atlas.cc
  vertex.cc
  render.cc
  font_ll.cc
  context.cc
  application.cc
  compatibility.cc)

if(NOT NANO_BROWSER)
  target_link_libraries(render_lib uwebsockets)
endif()

if(NANO_USE_FREETYPE)
  target_link_libraries(render_lib freetype)
endif()

add_executable(${RENDER_NAME}
  main.cc)

target_link_libraries(${RENDER_NAME} render_lib)
