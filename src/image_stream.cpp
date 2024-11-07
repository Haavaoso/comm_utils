//
// Created by havso on 07/11/2024.
//

#include "image_stream.hpp"


ImageCapturer::ImageCapturer(int cameraId, const std::string& windowTitle)
    : capture(cameraId), windowTitle(windowTitle) {
    if (!capture.isOpened()) {
        throw std::runtime_error("Unable to open camera");
    }
    cv::namedWindow(windowTitle, cv::WINDOW_AUTOSIZE);
    openWindows[windowTitle] = true;  // Track the main window
}


void ImageCapturer::displayImage(const cv::Mat& image) {
    if (!image.empty()) {
        cv::imshow(windowTitle, image);
        cv::waitKey(1);
    }
}

cv::Mat ImageCapturer::captureImage() {
    cv::Mat frame;
    capture >> frame;
    if (!frame.empty()) {
        cv::resize(frame, frame, cv::Size(1980 * 0.5, 1080 * 0.5), 0, 0, cv::INTER_NEAREST);
    }
    return frame;
}

bool ImageCapturer::isWindowClosed() const {
    return cv::getWindowProperty(windowTitle, cv::WND_PROP_VISIBLE) < 1;
}

void ImageCapturer::displayInNewWindow(const std::string& title, const cv::Mat& image) {
    if (!image.empty()) {
        // Create the window if it hasn't been created yet
        if (openWindows.find(title) == openWindows.end()) {
            cv::namedWindow(title, cv::WINDOW_AUTOSIZE);
            openWindows[title] = true;
        }
        cv::imshow(title, image);
        cv::waitKey(1);
    }
}