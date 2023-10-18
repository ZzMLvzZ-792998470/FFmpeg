#ifndef _DECODER_H
#define _DECODER_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
}

#include <memory>
//#include <deque>
//#include <mutex>
#include <condition_variable>
#include <memory>

#include "timer.h"
#include "ThreadSafeQueue.h"


class Decoder{
public:
    typedef std::shared_ptr<Decoder> ptr;
    Decoder(int set_framerate, AVFormatContext *ifmt_ctx);
    ~Decoder();

    //初始化解码器
    int init_decoder();

    //解码方法
    int decode(int& work);

    //补帧解码
    int decode_low2high();

    //丢帧解码
    int decode_high2low();

    //设置视频解码队列缓存
    void set_video_queue_cache(int& cache_size);

    //获得video_dec_ctx
    AVCodecContext *get_video_dec_ctx(){
        std::lock_guard<std::mutex> lock(para_mtx);
        return video_dec_ctx;
    }

    //获得audio_dec_ctx
    AVCodecContext *get_audio_dec_ctx(){
        std::lock_guard<std::mutex> lock(para_mtx);
        return audio_dec_ctx;
    }

    //获得视频解码队列
//    std::deque<AVFrame *> get_video_queue(){
//        std::lock_guard<std::mutex> lock(m_mtx);
//        return video_queue;
//    }
//    ThreadSafeDeque<AVFrame *> get_video_queue(){
//        return video_queue;
//    };


    //获得音频解码队列
//    std::deque<AVFrame *> get_audio_queue(){
//        std::lock_guard<std::mutex> lock(m_mtx);
//        return audio_queue;
//    }


    int test_decode();

    int test_decode_only_video(int& video_over);

    int test_decode_only_audio(int& audio_over);



    bool is_audio_queue_empty(){
        return audio_queue.empty();
    }


    bool is_video_queue_empty(){
        return video_queue.empty();
    }


    void clear_audio(){
        while(!audio_queue.empty()){
            av_frame_free(&audio_queue.back());
            audio_queue.pop_back();
        }
//        while(!audio_queue.empty()){
//            av_frame_free(&audio_queue.front());
//            audio_queue.pop_front();
//        }
    }


    void clear_video(){
        while(!video_queue.empty()){
            av_frame_free(&video_queue.back());
            video_queue.pop_back();
        }
//        while(!video_queue.empty()){
//            av_frame_free(&video_queue.front());
//            video_queue.pop_front();
//        }
    }


    //视频解码队列pop操作
    void pop_video(){
        av_frame_free(&video_queue.front());
        video_queue.pop_front();
    }


    //音频解码队列pop操作
    void pop_audio(){
        av_frame_free(&audio_queue.front());
        audio_queue.pop_front();
    }


    AVFrame* get_video_queue_front(){
        return video_queue.front();
    }

    AVFrame* get_audio_queue_front(){
        return audio_queue.front();
    }


    int change_fmt(AVFormatContext* fmt_ctx);

    int test_decode_high2low();

private:
    AVFormatContext *ifmt_ctx;

    double set_framerate;
    double real_framerate;

    int video_queue_cache = 25;
    int audio_queue_cache = 5;

    AVCodecContext *audio_dec_ctx = nullptr, *video_dec_ctx = nullptr;

    double frameratio = 0.0;
    double count = 0.0;

    AVFrame *prev_frame = nullptr;

    ThreadSafeDeque<AVFrame *> video_queue = {};
    ThreadSafeDeque<AVFrame *> audio_queue = {};


    bool is_changing = false;
    bool is_working = false;

    std::mutex m_mtx;
    std::mutex cv_mtx;
    std::mutex para_mtx;

    std::condition_variable cond;
};


#endif
