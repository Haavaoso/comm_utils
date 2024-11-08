//
// Created by havso on 07/11/2024.
//
#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

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

class WebSocketServer {
public:
    WebSocketServer(const std::string& host, uint16_t port)
        : host_(host), port_(port) {}

    // Start streaming the video from the provided VideoCapture object
    void startStream(cv::VideoCapture& videoStream) {
        try {
            // Set up WebSocket server
            auto const address = ip::make_address(host_);
            boost::asio::io_context ioc{};
            tcp::acceptor acceptor{ioc, {address, port_}};
            tcp::socket socket{ioc};

            acceptor.listen(1);
            std::cout << "Listening for WebSocket connections on ws://" << host_ << ":" << port_ << std::endl;

            // Accept a WebSocket connection
            acceptor.accept(socket);
            beast::websocket::stream<beast::tcp_stream> ws{std::move(socket)};
            ws.accept();
            ws.binary(true);

            // Check if the video stream is opened
            if (!videoStream.isOpened()) {
                std::cerr << "Error: Video stream is not open." << std::endl;
                return;
            }

            // Streaming loop
            while (true) {
                cv::Mat frame;
                videoStream >> frame;

                if (frame.empty()) {
                    std::cerr << "Error: Captured empty frame." << std::endl;
                    break;
                }

                // Encode the frame as JPEG
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

                // Control frame rate (e.g., ~30 FPS)
                std::this_thread::sleep_for(std::chrono::milliseconds(33));
            }

            // Close WebSocket connection
            ws.close(beast::websocket::close_code::normal);
            std::cout << "WebSocket connection closed." << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << '\n';
        }
    }

private:
    std::string host_;
    uint16_t port_;
};

#endif //WEBSOCKET_SERVER_HPP

