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
    Encoder(int height = 1080, int width = 1920, int framerate = 30, int samplerate = 44100, int encoder_type = 0);

    ~Encoder();

    //初始化编码器和编码上下文
    int init_encoder(AVFormatContext *ofmt_ctx);



    int init_encoder(AVFormatContext *ofmt_ctx, int type);


    int get_encoder_type() {return encoder_type;;}



    //flush清理最后残留数据
    int flush();

    //编码方法（默认最原始版本，模板）
    int encode(AVFrame *frame, int stream_index, double& count);

    //编码视频帧(有休眠时间,模板)
    int encode_video(AVFrame* frame, int64_t& last_time);

    //编码音频帧 模板
    int encode_audio(AVFrame* frame);


    //MP4格式编码视频帧
    int encode_video_mp4(AVFrame* frame);

    //MP4格式编码音频帧
    int encode_audio_mp4(AVFrame* frame);


    void setVideoEncCtxParam();

    void setAudioEncCtxParam();




    //获取video_enc_ctx
    AVCodecContext *get_video_enc_ctx(){return video_enc_ctx;}

    //获取audio_enc_ctx
    AVCodecContext *get_audio_enc_ctx(){return audio_enc_ctx;}


    //编码视频rtmp(已废弃)
    int encode_video_rtmp(AVFrame *frame, double& count_video, int64_t& last_time);

    //编码音频rtmp(已废弃)
    int encode_audio_rtmp(AVFrame *frame, double& count_audio, int64_t& last_time);


    //编码视频帧rtmp(计算每一帧编码间隔时间)
    int encode_video_rtmp(AVFrame *frame, int64_t& last_time);

    //编码音频帧rtmp(计算每一帧编码间隔时间)
    int encode_audio_rtmp(AVFrame* frame, int64_t& last_time);


    //编码视频帧rtmp(不需要时间和计数)
    int encode_video_rtmp(AVFrame *frame);

    //编码音频帧rtmp(不需要时间和计数)
    int encode_audio_rtmp(AVFrame* frame);


    //编码视频帧rtsp
    int encode_video_rtsp(AVFrame* frame, int64_t& last_time);

    //编码音频帧rtsp
    int encode_audio_rtsp(AVFrame* frame, int64_t& last_time);


    int encode_video_rtp(AVFrame* frame, int64_t& last_time);

    int encode_audio_rtp(AVFrame* frame, int64_t& last_time);




    int encode_video_test(AVFrame* frame, int64_t& last_time);


    int encode_audio_test(AVFrame* frame, int64_t& last_time);






private:
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

    int encoder_type = 0;


};




















#endif