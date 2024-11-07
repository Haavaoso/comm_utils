



add_executable(ws_server websocket_server_test.cpp)
target_link_libraries(ws_server PRIVATE Boost::system ${OpenCV_LIBS} simple_socket)
