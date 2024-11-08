//
// Created by havso on 07/11/2024.
//
#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <thread>
#include "websocket_server.hpp"

namespace beast = boost::beast;
namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;

int main() {
    // Open video source (0 for default camera or provide a file path)
    cv::VideoCapture videoStream(0);  // Replace "0" with a file path for a video file

    // Check if video source is opened
    if (!videoStream.isOpened()) {
        std::cerr << "Error: Unable to open video source." << std::endl;
        return 1;
    }

    // Initialize WebSocket server to stream video
    WebSocketServer server("0.0.0.0", 9002);
    server.startStream(videoStream);

    return 0;
}