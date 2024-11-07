//
// Created by havso on 07/11/2024.
//

#include "Image_stream.hpp"
#include <opencv2/opencv.hpp>


int main() {
    try {
        ImageCapturer capturer(0, "Camera Feed");

        while (true) {
            cv::Mat frame = capturer.captureImage();
            capturer.displayImage(frame);

            if (capturer.isWindowClosed()) {
                break;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}