cmake_minimum_required(VERSION 3.28)
project(comm_utils)

set(CMAKE_CXX_STANDARD 20)

# Include third-party dependencies if needed
include(FetchContent)
FetchContent_Declare(
        SimpleSocket
        GIT_REPOSITORY https://github.com/markaren/SimpleSocket.git
        GIT_TAG e0e989bdd52137d743a4599e25167d5681a0de00
)
FetchContent_MakeAvailable(SimpleSocket)

find_package(OpenCV REQUIRED)

include_directories(include)

add_executable(main main.cpp)
add_executable(image_stream_test tests/image_stream_test.cpp
        src/image_stream.cpp)

add_executable(tcp_client_test tests/tcp_client_test.cpp)

add_executable(tcp_server_test tests/tcp_server_test.cpp
        src/image_stream.cpp)
add_executable(websocket_server_test tests/websocket_server_test.cpp)


target_link_libraries(image_stream_test PRIVATE ${OpenCV_LIBS})
target_link_libraries(tcp_client_test PRIVATE ${OpenCV_LIBS} simple_socket)
target_link_libraries(tcp_server_test PRIVATE ${OpenCV_LIBS} simple_socket)
target_link_libraries(websocket_server_test PRIVATE ${OpenCV_LIBS} simple_socket)


add_subdirectory(tests PRIVATE)
