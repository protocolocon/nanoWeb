set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -lssl -lcrypto -lz")


include_directories(
  ${PROJECT_SOURCE_DIR}/src/server
  ${PROJECT_SOURCE_DIR}/src/protocol
  ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src)

# server
add_executable(application
  main.cc)

target_link_libraries(application server_lib)
