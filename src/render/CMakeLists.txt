if(NANO_BROWSER)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_FILESYSTEM=0 --memory-init-file 0") # for SOCKFS, no independent .mem
else()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -lssl -lcrypto -lz")

  include_directories(
    ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src)
endif()

include_directories(
  ${PROJECT_SOURCE_DIR}/src/protocol
  ${PROJECT_SOURCE_DIR}/submodules/nanovg/src)

# render
add_library(render_lib
  font.cc
  atlas.cc
  vertex.cc
  render.cc
  context.cc
  application.cc
  compatibility.cc)

if(NOT NANO_BROWSER)
  target_link_libraries(render_lib uwebsockets)
endif()

add_executable(${RENDER_NAME}
  main.cc)

target_link_libraries(${RENDER_NAME} render_lib)
