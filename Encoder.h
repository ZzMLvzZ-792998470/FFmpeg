#ifndef _ENCODER_H
#define _ENCODER_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
}


#include "Writer.h"
#include "Timer.h"
#include "Distribute.h"
#include "FrameCreater.h"

#include <string>
#include <memory>
#include <condition_variable>


class Encoder{
public:
    typedef std::shared_ptr<Encoder> ptr;
    Encoder(int width = 1920,
            int height = 1080,
            AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P,
            int framerate = 30,
            uint64_t channel_layout = 3,
            AVSampleFormat sample_fmt = AV_SAMPLE_FMT_FLTP,
            int samplerate = 44100);

    ~Encoder();

    //初始化视频编码器
    int init_video_encoder(AVFormatContext* ofmt_ctx);

    //初始化音频编码器
    int init_audio_encoder(AVFormatContext* ofmt_ctx);

    //编码视频帧
    int encode_video(Distributer::ptr distributer, AVFrame* frame, int64_t& last_time);

    //编码音频帧
    int encode_audio(Distributer::ptr distributer, AVFrame* frame, int64_t& last_time);

    //编码视频帧
    int encode_video(Distributer::ptr distributer, AVFrame* frame);

    //编码音频帧
    int encode_audio(Distributer::ptr distributer, AVFrame* frame);


    //获取视频编码器上下文
    AVCodecContext *get_video_enc_ctx() const {return video_enc_ctx;}

    //获取音频编码器上下文
    AVCodecContext *get_audio_enc_ctx() const {return audio_enc_ctx;}


    double get_time_video(){return current_time_video;}

    double get_time_audio(){return current_time_audio;}


    int encode_video_without_sleep(Distributer::ptr distributer);

    int encode_audio_without_sleep(Distributer::ptr dsitributer);

    void synchronize(Distributer::ptr distributer);


private:
    int width;
    int height;
    AVPixelFormat pix_fmt;
    int framerate;


    uint64_t channel_layout;
    AVSampleFormat sample_fmt;
    int samplerate;
    AVCodecContext *audio_enc_ctx = nullptr, *video_enc_ctx = nullptr;

    double current_time_video = 0.0;
    double current_time_audio = 0.0;

    double video_pts = 0.0;
    double audio_pts = 0.0;

    double time_per_frame_video;
    double time_per_frame_audio;

    int64_t last_video_time = 0;
    int64_t last_audio_time = 0;

    int64_t standard_video_time = 0;
    int64_t standard_audio_time = 0;


    std::mutex v_mtx;
    std::mutex a_mtx;
};


#endif