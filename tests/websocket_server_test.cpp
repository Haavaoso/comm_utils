//
// Created by havso on 07/11/2024.
//
#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <thread>
#include <chrono>

namespace beast = boost::beast;
namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;

int main() {
    try {
        // Set up the WebSocket server
        auto const address = ip::make_address("0.0.0.0");
        boost::asio::ip::port_type port{9002};  // Use port 9002 as per your HTML client

        boost::asio::io_context ioc{};
        tcp::acceptor acceptor{ioc, {address, port}};
        tcp::socket socket{ioc};

        acceptor.listen(1);
        std::cout << "Listening for WebSocket connections on ws://localhost:9002" << std::endl;

        // Accept a connection
        acceptor.accept(socket);
        beast::websocket::stream<beast::tcp_stream> ws{std::move(socket)};
        ws.accept();

        // Open a video stream (e.g., from the camera or a video file)
        cv::VideoCapture videoStream(0);  // Use "0" for the default camera, or replace with a file path
        if (!videoStream.isOpened()) {
            std::cerr << "Error: Unable to open video stream." << std::endl;
            return 1;
        }

        ws.binary(true);  // Set WebSocket stream to binary mode

        // Streaming loop
        while (true) {
            cv::Mat frame;
            videoStream >> frame;  // Capture a frame

            if (frame.empty()) {
                std::cerr << "Error: Captured empty frame." << std::endl;
                break;
            }

            // Encode frame as JPEG
            std::vector<uchar> buffer;
            if (!cv::imencode(".jpg", frame, buffer)) {
                std::cerr << "Error: Failed to encode frame." << std::endl;
                continue;
            }

            // Send the encoded frame over WebSocket
            try {
                ws.write(boost::asio::buffer(buffer.data(), buffer.size()));
            } catch (const beast::system_error& e) {
                std::cerr << "Error: WebSocket write failed - " << e.what() << std::endl;
                break;
            }

            // Add a small delay to control the frame rate (optional)
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }

        ws.close(beast::websocket::close_code::normal);  // Close WebSocket connection
        std::cout << "WebSocket connection closed." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}