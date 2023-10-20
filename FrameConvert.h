#ifndef _FRAMECONVERT_H
#define _FRAMECONVERT_H

extern "C"{
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/channel_layout.h>
};


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

/*
 * 传入数据帧 判断其类型 并转换为统一的格式 存储在 音视频队列中
 *
 *
 *
 * */


class FrameConverter{
public:
    typedef std::shared_ptr<FrameConverter> ptr;
    FrameConverter(int width,
                   int height,
                   AVPixelFormat pix_fmt,
                   uint64_t channel_layout,
                   int sample_rate,
                   AVSampleFormat sample_format);
    ~FrameConverter();


    int init_converter(uint64_t src_channel_layout,
                       int src_sample_rate,
                       AVSampleFormat src_sample_fmt);

    int reset_converter(uint64_t src_channel_layout, int src_sample_rate, AVSampleFormat src_sample_fmt);


    AVFrame* convert(AVFrame* frame);


    static AVFrame* change_frame_size(int width, int height, AVFrame* frame);


    static AVFrame* merge_frames(int backround_width, int backround_height, std::vector<AVFrame* >& frames);


    static cv::Mat avframe2Mat(int width, int height, AVFrame *frame);

    //Mat -----> AVFrame
    static AVFrame* Mat2avframe(int& cols, int& height, cv::Mat img);


    static void add_pic2pic(cv::Mat& src_mat, cv::Mat& added_mat,
                     int col_point = 200, int row_point = 200,
                     double alpha = 0.1, double beta = 0.9, double gamma = 1.0);



private:
    //目标参数
    int width = 1920;
    int height = 1080;
    AVPixelFormat pixel_fmt = AV_PIX_FMT_YUV420P;

    uint64_t channel_layout = 3;
    int sample_rate = 44100;
    AVSampleFormat sample_fmt = AV_SAMPLE_FMT_FLTP;

    SwrContext *swr_ctx = nullptr;
    AVAudioFifo *fifo = nullptr;
};


















#endif
