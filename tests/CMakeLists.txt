
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec libavformat libavutil libswscale)



add_executable(ws_server websocket_server_test.cpp)
target_link_libraries(ws_server PRIVATE Boost::system ${OpenCV_LIBS} simple_socket)

add_executable(udp udp_h264_test.cpp)
target_link_libraries(udp PRIVATE Boost::system ${OpenCV_LIBS} simple_socket PkgConfig::FFMPEG)
#