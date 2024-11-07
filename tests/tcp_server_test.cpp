//
// Created by havso on 07/11/2024.
//

#include "image_stream.hpp"
#include "tcp_server.hpp"

std::atomic<bool> running(true);
cv::Mat image{};
std::mutex imageMutex;
int main() {
    try {
        uint16_t port = 8080;
        ImageCapturer capturer;
        tcp_server server(port);

        server.startServer();
        std::cout << "lool" << std::endl;
        // Main loop to capture and process images
        while (running) {

            cv::Mat frame = capturer.captureImage();
            capturer.displayImage(frame);
            if (!frame.empty()) {
                std::lock_guard<std::mutex> lock(imageMutex);
                image = frame.clone();
            }



            server.updateFrame(frame);

            cv::Mat receivedImage = server.getReceivedImage();

            if (!receivedImage.empty()) {
                capturer.displayInNewWindow("Secondary View", receivedImage);
            }

            if (cv::waitKey(1) == 'q') {
                server.stopServer();
                break;
            }
        }


    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}