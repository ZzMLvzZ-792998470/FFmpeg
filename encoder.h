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
#include <string>
#include <memory>


class Encoder{
public:
    typedef std::shared_ptr<Encoder> ptr;
    Encoder(int height = 1080, int width = 1920, int framerate = 30, int samplerate = 44100);

    ~Encoder();

    //初始化编码器和编码上下文
    int init_encoder(AVFormatContext *ofmt_ctx);

    //flush清理最后残留数据
    int flush();

    //编码方法（默认）
    int encode(AVFrame *frame, int stream_index, double& count);

    //编码视频帧(有休眠时间)
    int encode_video(AVFrame* frame, int64_t& last_time);

    //MP4格式编码视频帧
    int encode_video_mp4(AVFrame* frame);

    //编码音频帧
    int encode_audio(AVFrame* frame);

    //获取video_enc_ctx
    AVCodecContext *get_video_enc_ctx(){return video_enc_ctx;}

    //获取audio_enc_ctx
    AVCodecContext *get_audio_enc_ctx(){return audio_enc_ctx;}



    //测试编码视频
    int encode_new(AVFrame *frame, int stream_index, double& count, int64_t& last_time);

    //测试编码
    int test_encode(AVFrame* frame, int stream_index, double& count, int64_t & time, int& first, int& resend);


    void setVideoEncCtxParam();

    void setAudioEncCtxParam();



private:
   // Writer::ptr writer;
    AVFormatContext *m_ofmt_ctx;

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