cmake_minimum_required(VERSION 4.0)

add_executable(server src/server/server.cpp src/server/room.cpp src/server/chat_session.cpp)
add_executable(client src/client/main_client.cpp src/client/client.cpp)

set(MY_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MY_OUTPUT_DIR})

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${MY_OUTPUT_DIR}
)

set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${MY_OUTPUT_DIR}
)
