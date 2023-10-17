#ifndef _AUDIO_RESAMPLER_H
#define _AUDIO_RESAMPLER_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
};

#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>



class AudioResampler{
public:
    typedef std::shared_ptr<AudioResampler> ptr;
    AudioResampler();
    ~AudioResampler();


    int init_swr(AVCodecContext* audio_dec_ctx, AVCodecContext* audio_enc_ctx);

    int init_fifo(AVCodecContext* audio_enc_ctx);

    int write_to_fifo(AVFrame* frame, AVCodecContext* audio_enc_ctx, int dst_sample_rate);

    AVFrame* get_from_fifo(AVCodecContext* audio_enc_ctx);


    void clear_fifo();
    void clear_swr();

    int get_fifo_size();

    int reset_resampler(AVCodecContext* audio_dec_ctx, AVCodecContext * audio_enc_ctx);


private:
    SwrContext *swr_ctx = nullptr;
    AVAudioFifo *fifo = nullptr;

    std::mutex m_mtx;
    std::condition_variable cond;
    bool fifo_is_working = false;
};








#endif
