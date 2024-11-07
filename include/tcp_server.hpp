//
// Created by havso on 07/11/2024.
//

#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <opencv2/opencv.hpp>
#include "network_helper.hpp"
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include "simple_socket/TCPSocket.hpp"
#include <iostream>


class tcp_server {
public:
    explicit tcp_server(const uint16_t port)
        : tcpServer(port), stopFlag(false) {}

    void startServer() {
        connectionThread = std::jthread([this] {
            while (!stopFlag) {
                auto connection = tcpServer.accept();
                if (!connection || stopFlag) break;
                std::cout << "Client connected" << std::endl;
                clientThreads.emplace_back(&tcp_server::clientHandler, this, std::move(connection));
            }
        });
    }

    void stopServer() {
        stopFlag = true;
        if (connectionThread.joinable()) connectionThread.join();
        for (auto& thread : clientThreads) {
            if (thread.joinable()) thread.join();
        }
    }

    cv::Mat getReceivedImage() {
        std::lock_guard<std::mutex> lock(imageMutex);
        return receivedImage.clone();
    }


    static void sendRawData(const std::unique_ptr<simple_socket::SimpleConnection>& connection, const std::vector<unsigned char>& data) {
        connection->write(data);
    }

    static void sendMatData(const std::unique_ptr<simple_socket::SimpleConnection>& connection, const cv::Mat& frame) {
        const auto frame_encoded = cvMat2uchar(frame);
        connection->write(frame_encoded);
    }

    void updateFrame(const cv::Mat& frame) {
        currentImage = frame;
    }

    static std::vector<unsigned char> cvMat2uchar(const cv::Mat& image) {
        std::vector<uchar> img_buffer;
        cv::imencode(".jpg", image, img_buffer);
        auto test = img_buffer;


        const int size = static_cast<int>(img_buffer.size());

        const auto bytes = int_to_bytes(size, byte_order::BIG);

        std::vector<unsigned char> toSend(img_buffer.begin(), img_buffer.end());
        toSend.insert(toSend.begin(), bytes.begin(), bytes.end());

        return toSend;
    }




private:
    simple_socket::TCPServer tcpServer;
    std::atomic<bool> stopFlag;
    std::jthread connectionThread;
    std::vector<std::jthread> clientThreads;
    cv::Mat currentImage;
    cv::Mat receivedImage;
    std::mutex imageMutex;

    void clientHandler(std::unique_ptr<simple_socket::SimpleConnection> connection) {
        while (!stopFlag) {
            cv::Mat imageToSend;
            {
                std::lock_guard<std::mutex> lock(imageMutex);
                if (currentImage.empty()) break;
                imageToSend = currentImage.clone();
            }

            auto encodedImage = encodeImage(imageToSend);
            sendData(connection, encodedImage);
            std::cout << "Sent message." << std::endl;



            auto clientData = receiveData(connection);
            if (clientData.empty()) {
                std::cout << "No dataaa." << std::endl;
                break;
            } try {
                cv::Mat clientImage = uChar2cvMat(clientData);
            {
                std::lock_guard<std::mutex> lock(imageMutex);
                receivedImage = clientImage.clone();
                std::cout << "Recieved message back." << std::endl;
            }
            } catch (const cv::Exception& e) {
                std::cerr << "Error decoding image: " << e.what() << std::endl;
                break;
            }
        }
    }

    static std::vector<unsigned char> encodeImage(const cv::Mat& img) {
        std::vector<unsigned char> buffer;
        cv::imencode(".jpg", img, buffer);
        auto sizeBytes = int_to_bytes(static_cast<int>(buffer.size()), byte_order::BIG);
        buffer.insert(buffer.begin(), sizeBytes.begin(), sizeBytes.end());
        return buffer;
    }

    static void sendData(const std::unique_ptr<simple_socket::SimpleConnection>& connection, const std::vector<unsigned char>& data) {
        connection->write(data);
    }

    static std::vector<unsigned char> receiveData(const std::unique_ptr<simple_socket::SimpleConnection>& connection) {
        std::vector<unsigned char> sizeBuffer(4);
        connection->readExact(sizeBuffer);
        int msgSize = bytes_to_int(sizeBuffer, byte_order::BIG);
        std::vector<unsigned char> message(msgSize);
        connection->readExact(message);
        return message;
    }

    static cv::Mat uChar2cvMat(const std::vector<unsigned char>& imgData) {
        return cv::imdecode(imgData, cv::IMREAD_COLOR);
    }
};
#endif //TCP_SERVER_HPP
