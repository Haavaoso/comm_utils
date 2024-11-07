add_executable(tcp_client_test tcp_client_test.cpp)
add_executable(tcp_server_test tcp_server_test.cpp)
add_executable(websocket_server_test websocket_server_test.cpp)

target_link_libraries(tcp_client_test PRIVATE ${OpenCV_LIBS} simple_socket)
target_link_libraries(tcp_server_test PRIVATE ${OpenCV_LIBS} simple_socket)
target_link_libraries(websocket_server_test PRIVATE ${OpenCV_LIBS} simple_socket)
