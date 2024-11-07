#ifndef IMAGE_STREAM_HPP
#define IMAGE_STREAM_HPP

#include <opencv2/opencv.hpp>



// ImageCapturer class that manages capturing, resizing, and displaying images
class ImageCapturer {
public:
    explicit ImageCapturer(int cameraId = 0, const std::string& windowTitle = "Captured Image");

    cv::Mat captureImage();

    void displayImage(const cv::Mat& image);

    [[nodiscard]] bool isWindowClosed() const;

    void displayInNewWindow(const std::string& title, const cv::Mat& image);

private:
    cv::VideoCapture capture;
    std::string windowTitle;
    std::unordered_map<std::string, bool> openWindows;

};


#endif //IMAGE_STREAM_HPP