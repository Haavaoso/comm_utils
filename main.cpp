//
// Created by havso on 07/11/2024.
//

#include <opencv2/opencv.hpp>


int main() {
    cv::VideoCapture cap("udp://@10.25.47.55:9090");  // Use 0 for the default webcam. Replace with a file path if needed.


    while (true)
    {
        cv::Mat frame;
        // Capture a frame from the video source
        bool success = cap.read(frame);

        // Check if the frame was captured successfully
        if (!success) {
            std::cerr << "Error: Could not read a frame from the video source." << std::endl;
            break;
        }

        // Display the captured frame
        cv::imshow("H264 Stream", frame);

        // Press 'ESC' to exit the loop
        if (cv::waitKey(1) == 27) {
            break;
        }
    }
}