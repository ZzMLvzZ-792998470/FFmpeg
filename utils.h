#ifndef _UTILS_H
#define _UTILS_H


extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
}

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>


class Utils{
public:
    //合并所有视频帧到一个帧并输出
    static AVFrame* merge_way(int backround_width, int backround_height, std::vector<AVFrame *>& frames);

    //改变数据帧画面大小
    static AVFrame* change_frame_size(int new_width, int new_height, AVFrame *src_frame);

    //AVFrame ----> Mat
    static cv::Mat avframe2Mat(int width, int height, AVFrame *frame);

    //Mat -----> AVFrame
    static AVFrame* Mat2avframe(int& cols, int& height, cv::Mat img);

    //叠加图片
    static void add_pic2pic(cv::Mat& src_mat, cv::Mat& added_mat,
                     int col_point = 200, int row_point = 200,
                     double alpha = 0.1, double beta = 0.9, double gamma = 1.0);


    //写音频数据到fifo
    static int write_to_fifo(AVAudioFifo* fifo, SwrContext* swr_ctx, AVFrame* src_frame, int channels, AVSampleFormat sample_fmt, int dst_sample_rate);

    //获得fifo中 处理之后的音频数据帧
    static AVFrame* get_sample_fixed_frame(AVAudioFifo* fifo, AVCodecContext* audio_enc_ctx, bool is_last_data);

};

#endif
