set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -lssl -lcrypto -lz")


include_directories(
  ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src)

# µws
add_library(uwebsockets
  ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src/Extensions.cpp
  ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src/Group.cpp
  ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src/Networking.cpp
  ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src/Hub.cpp
  ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src/Node.cpp
  ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src/WebSocket.cpp
  ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src/HTTPSocket.cpp
  ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src/Socket.cpp
  ${PROJECT_SOURCE_DIR}/submodules/uWebSockets/src/Epoll.cpp)

# server
add_executable(server
  main.cc
  server.cc)

target_link_libraries(server uwebsockets)
