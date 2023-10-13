#ifndef _ENCODER_H
#define _ENCODER_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
}


#include "writer.h"
#include "timer.h"
#include "distribute.h"

#include <string>
#include <memory>


class Encoder{
public:
    typedef std::shared_ptr<Encoder> ptr;
    Encoder(int height = 1080, int width = 1920, int framerate = 30, int samplerate = 44100);

    ~Encoder();

    //初始化视频编码器
    int init_video_encoder(AVFormatContext* ofmt_ctx);

    //初始化音频编码器
    int init_audio_encoder(AVFormatContext* ofmt_ctx);

    //编码视频帧
    int encode_video(Distributer::ptr distributer, AVFrame* frame, int64_t& last_time);

    //编码音频帧
    int encode_audio(Distributer::ptr distributer, AVFrame* frame, int64_t& last_time);


    //获取视频编码器上下文
    AVCodecContext *get_video_enc_ctx() const {return video_enc_ctx;}

    //获取音频编码器上下文
    AVCodecContext *get_audio_enc_ctx() const {return audio_enc_ctx;}


private:
    int height = 1080;
    int width = 1920;
    int framerate = 30;
    int samplerate = 44100;
    AVCodecContext *audio_enc_ctx = nullptr, *video_enc_ctx = nullptr;

    double current_time_video = 0.0;
    double current_time_audio = 0.0;

    double video_pts = 0.0;
    double audio_pts = 0.0;

    double time_per_frame_video;
    double time_per_frame_audio;
};


#endif