#ifndef _DECODER_H
#define _DECODER_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
}

#include <memory>
#include <vector>
#include <memory>

#include "Timer.h"
#include "ThreadSafeQueue.h"
#include "FrameConvert.h"

class Decoder{
public:
    typedef std::shared_ptr<Decoder> ptr;

    //构造函数 持有ifmt_ctx, converter
    Decoder(int set_framerate, AVFormatContext *ifmt_ctx, FrameConverter::ptr converter);

   //析构函数
    ~Decoder();

    //初始化解码器
    int init_decoder();

    //解码方法
    int decode(int& work);

    //补帧解码
    int decode_low2high();

    //丢帧解码
    int decode_high2low();

    //获得video_dec_ctx
    AVCodecContext *get_video_dec_ctx() const;

    //获得audio_dec_ctx
    AVCodecContext *get_audio_dec_ctx() const;

    //使用音频队列
    void use_audio();

    bool is_using_audio();

    //获取音频队列front
    AVFrame *get_audio_front();

    //获取视频队列front
    AVFrame *get_video_front();

    //弹出音频队列front
    void pop_audio_front();

    //弹出视频队列front
    void pop_video_front();

    //弹出音频队列back
    void pop_audio_back();

    //弹出视频队列back
    void pop_video_back();

    //判断视频队列是否为空
    bool is_audio_empty();

    //判断音频队列是否为空
    bool is_video_empty();

    //切流时改变fmt_ctx
    int change_fmt(AVFormatContext* fmt_ctx);

    //获取音频队列头元素并移除
    AVFrame* get_audio();

    //获取视频队列头元素并移除
    AVFrame* get_video();

    //清理音频队列
    void clear_audio();

    //清理视频队列
    void clear_video();



private:
    AVFormatContext *ifmt_ctx;
    FrameConverter::ptr converter;


    double set_framerate;
    double real_framerate;

    AVCodecContext *audio_dec_ctx = nullptr, *video_dec_ctx = nullptr;

    double frameratio = 0.0;
    double count = 0.0;

    int video_queue_cache = 15;
    int audio_queue_cache = 30;

    AVFrame *prev_frame = nullptr;
    bool using_audio = false;

    ThreadSafeDeque<AVFrame* > video_queue = {};
    ThreadSafeDeque<AVFrame* > audio_queue = {};

    std::mutex m_mtx;
    std::mutex a_mtx;
    std::mutex v_mtx;

    bool is_changing = false;
};


#endif
