if(NANO_BROWSER)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_FILESYSTEM=1 --memory-init-file 1")
endif()


include_directories(
  ${PROJECT_SOURCE_DIR}/submodules/nanovg/src)

# nanoWeb library
add_library(nanoweb
  util.cc
  types.cc
  input.cc
  action.cc
  widget.cc
  render.cc
  context.cc
  ml_parser.cc
  type_widget.cc
  application.cc
  widget_timer.cc
  compatibility.cc
  widget_layout.cc
  string_manager.cc
  reserved_words.cc
  widget_template.cc
  widget_application.cc
  ${PROJECT_SOURCE_DIR}/submodules/nanovg/src/nanovg.c)

add_executable(${NANOWEB_NAME}
  main.cc)

target_link_libraries(${NANOWEB_NAME} nanoweb)
