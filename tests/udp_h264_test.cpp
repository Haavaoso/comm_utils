extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int main() {
    // Initialize FFmpeg
    avformat_network_init();

    // Open UDP stream with FFmpeg
    AVFormatContext* format_context = avformat_alloc_context();
    if (!format_context) {
        std::cerr << "Could not allocate format context." << std::endl;
        return -1;
    }

    // Open the UDP input stream
    if (avformat_open_input(&format_context, "udp://0.0.0.0:9090", NULL, NULL) != 0) {
        std::cerr << "Could not open input stream." << std::endl;
        avformat_free_context(format_context);
        return -1;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(format_context, NULL) < 0) {
        std::cerr << "Could not find stream information." << std::endl;
        avformat_close_input(&format_context);
        return -1;
    }

    // Find the first video stream
    const AVCodec* codec = nullptr;
    AVCodecParameters* codec_params = nullptr;
    int video_stream_index = -1;

    for (unsigned int i = 0; i < format_context->nb_streams; i++) {
        AVStream* stream = format_context->streams[i];
        codec_params = stream->codecpar;
        codec = avcodec_find_decoder(codec_params->codec_id);

        if (codec && codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1) {
        std::cerr << "Could not find a video stream." << std::endl;
        avformat_close_input(&format_context);
        return -1;
    }

    // Set up codec context
    AVCodecContext* codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        std::cerr << "Could not allocate codec context." << std::endl;
        avformat_close_input(&format_context);
        return -1;
    }

    if (avcodec_parameters_to_context(codec_context, codec_params) < 0) {
        std::cerr << "Could not initialize codec context." << std::endl;
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    if (avcodec_open2(codec_context, codec, NULL) < 0) {
        std::cerr << "Could not open codec." << std::endl;
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    // Allocate frames and packet for decoding
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* rgb_frame = av_frame_alloc();
    if (!packet || !frame || !rgb_frame) {
        std::cerr << "Could not allocate frame or packet." << std::endl;
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    int width = codec_context->width;
    int height = codec_context->height;
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, width, height, 32);
    std::vector<uint8_t> buffer(num_bytes);
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer.data(), AV_PIX_FMT_BGR24, width, height, 1);

    // Set up SWS context for frame conversion
    SwsContext* sws_context = sws_getContext(width, height, codec_context->pix_fmt,
                                             width, height, AV_PIX_FMT_BGR24,
                                             SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_context) {
        std::cerr << "Could not initialize SWS context." << std::endl;
        av_packet_free(&packet);
        av_frame_free(&frame);
        av_frame_free(&rgb_frame);
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    // OpenCV window
    cv::namedWindow("H264 Stream", cv::WINDOW_AUTOSIZE);

    // Variables for FPS calculation
    int frame_count = 0;
    double fps = 0.0;
    auto start_time = std::chrono::steady_clock::now();

    // Start reading packets from the stream
    while (av_read_frame(format_context, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            if (avcodec_send_packet(codec_context, packet) == 0) {
                while (avcodec_receive_frame(codec_context, frame) == 0) {
                    // Convert the frame to BGR format for OpenCV
                    sws_scale(sws_context, frame->data, frame->linesize, 0, height,
                              rgb_frame->data, rgb_frame->linesize);

                    // Wrap the RGB frame data in an OpenCV Mat
                    cv::Mat img(height, width, CV_8UC3, rgb_frame->data[0], rgb_frame->linesize[0]);

                    // FPS Calculation
                    frame_count++;
                    auto end_time = std::chrono::steady_clock::now();
                    double elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
                    if (elapsed_seconds >= 1.0) {
                        fps = frame_count / elapsed_seconds;
                        frame_count = 0;
                        start_time = end_time;
                    }

                    // Display FPS on the frame
                    std::string fps_text = "FPS: " + std::to_string(static_cast<int>(fps));
                    cv::putText(img, fps_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);

                    // Display the frame
                    cv::imshow("H264 Stream", img);

                    // Exit on ESC key
                    if (cv::waitKey(1) == 27) {
                        av_packet_unref(packet);
                        goto end;
                    }
                }
            }
        }
        av_packet_unref(packet);
    }

end:
    // Clean up resources
    sws_freeContext(sws_context);
    av_packet_free(&packet);
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    avcodec_free_context(&codec_context);
    avformat_close_input(&format_context);
    avformat_network_deinit();

    cv::destroyAllWindows();
    return 0;
}