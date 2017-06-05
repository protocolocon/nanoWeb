if(NOT NANO_BROWSER)
  # server part
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -lssl -lcrypto -lz")

  include_directories(
    ${PROJECT_SOURCE_DIR}/src/server
    ${PROJECT_SOURCE_DIR}/src/protocol
    ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src)

  # server
  add_executable(server_basic
    server.cc)

  target_link_libraries(server_basic server_lib)

  set(CLIENT_NAME "client_basic")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_FILESYSTEM=0 --memory-init-file 0") # for SOCKFS, no independent .mem
  set(CLIENT_NAME "client_basic.js")
endif()

include_directories(
  ${PROJECT_SOURCE_DIR}/src/client
  ${PROJECT_SOURCE_DIR}/src/protocol)

# client part
add_executable(${CLIENT_NAME}
  client.cc)

target_link_libraries(${CLIENT_NAME} client_lib)

