//
// Created by havso on 07/11/2024.
//
#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include "simple_socket/WebSocket.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>

class WebSocketServer {
public:
    WebSocketServer(const std::string& host, uint16_t port)
        : websocketClient(), host_(host), port_(port), connected_(false) {

        // Set up WebSocket callbacks
        websocketClient.onOpen = [this](simple_socket::WebSocketConnection* conn) {
            if (conn) {
                std::cout << "WebSocket connection opened: " << conn->uuid() << std::endl;
                connected_ = true;
            } else {
                std::cerr << "Failed to open WebSocket connection" << std::endl;
            }
        };

        websocketClient.onClose = [this](simple_socket::WebSocketConnection* conn) {
            if (conn) {
                std::cout << "WebSocket connection closed: " << conn->uuid() << std::endl;
            }
            connected_ = false;
        };

        websocketClient.onMessage = [](simple_socket::WebSocketConnection* conn, const std::string& msg) {
            if (conn) {
                std::cout << "Message received from server: " << msg << std::endl;
            }
        };

        try {
            websocketClient.connect(host, port);
        } catch (const std::exception& e) {
            std::cerr << "Exception during WebSocket connect: " << e.what() << std::endl;
        }

        waitForConnection();
    }

    // Method to convert and send cv::Mat over WebSocket
    void sendMat(const cv::Mat& mat) {
        if (!connected_) {
            std::cerr << "WebSocket is not connected. Cannot send data." << std::endl;
            return;
        }

        if (mat.empty()) {
            std::cerr << "Empty image. Nothing to send." << std::endl;
            return;
        }

        try {
            // Encode cv::Mat to JPEG format
            std::vector<uchar> buffer;
            cv::imencode(".jpg", mat, buffer);

            // Convert encoded buffer to a string for transmission
            std::string encodedData(buffer.begin(), buffer.end());

            // Send the encoded image data through WebSocket
            websocketClient.send(encodedData);
        } catch (const std::exception& e) {
            std::cerr << "Exception during sendMat: " << e.what() << std::endl;
        }
    }

    // Close the WebSocket connection
    void close() {
        if (connected_) {
            try {
                websocketClient.close();
                connected_ = false;
            } catch (const std::exception& e) {
                std::cerr << "Exception during close: " << e.what() << std::endl;
            }
        }
    }

private:
    simple_socket::WebSocketClient websocketClient;
    std::string host_;
    uint16_t port_;
    bool connected_;

    // Helper method to wait for the WebSocket connection to establish
    void waitForConnection() {
        int retryCount = 0;
        while (!connected_ && retryCount < 10) {
            std::cout << "Waiting for WebSocket connection..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            retryCount++;
        }

        if (!connected_) {
            std::cerr << "Failed to establish WebSocket connection." << std::endl;
        }
    }
};

#endif //WEBSOCKET_SERVER_HPP

