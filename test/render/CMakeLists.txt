set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -lssl -lcrypto -lz")

include_directories(
  ${PROJECT_SOURCE_DIR}/src/render
  ${PROJECT_SOURCE_DIR}/submodules/Catch/include)

add_executable(test_render
  test.cc
  test_atlas.cc)

target_link_libraries(test_render render_lib)
