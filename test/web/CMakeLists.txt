set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

include_directories(
  ${PROJECT_SOURCE_DIR}/src/web
  ${PROJECT_SOURCE_DIR}/submodules/nanovg/src
  ${PROJECT_SOURCE_DIR}/submodules/Catch/include)

add_executable(test_nanoweb
  test.cc
  test_action.cc
  test_parser.cc
  test_application.cc)

target_link_libraries(test_nanoweb nanoweb)
