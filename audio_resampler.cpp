#include "audio_resampler.h"

AudioResampler::AudioResampler() {

}


AudioResampler::~AudioResampler() {
    swr_free(&swr_ctx);
    swr_ctx = nullptr;

    av_audio_fifo_free(fifo);
    fifo = nullptr;
}


int AudioResampler::init_swr(AVCodecContext *audio_dec_ctx, AVCodecContext *audio_enc_ctx) {
    int ret;

    {
        std::lock_guard<std::mutex> lock(swr_mtx);
        swr_ctx = swr_alloc_set_opts(nullptr,
                                     audio_enc_ctx->channel_layout,
                                     audio_enc_ctx->sample_fmt,
                                     audio_enc_ctx->sample_rate,
                                     audio_dec_ctx->channel_layout,
                                     audio_dec_ctx->sample_fmt,
                                     audio_dec_ctx->sample_rate,
                                     0,
                                     nullptr);

        ret = swr_init(swr_ctx);
    }
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "swr_ctx init failed.\n");
        return -1;
    }

    return 0;
}


int AudioResampler::init_fifo(AVCodecContext* audio_enc_ctx) {
    int ret;

    {
        std::lock_guard<std::mutex> locK(fifo_mtx);
        fifo = av_audio_fifo_alloc(audio_enc_ctx->sample_fmt,
                                   audio_enc_ctx->channels,
                                   1);
        if (!fifo) {
            av_log(nullptr, AV_LOG_ERROR, "fifo init wrong.\n");
            return -1;
        }
    }
    return ret;
}


int AudioResampler::write_to_fifo(AVFrame *frame, AVCodecContext *audio_enc_ctx, int dst_sample_rate) {
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        while(is_changing){
            cond.wait(lock);
        }
        is_working = true;
    }


    uint8_t **audio_data_buffer = nullptr;
    av_samples_alloc_array_and_samples(&audio_data_buffer,
                                       nullptr,
                                       audio_enc_ctx->channels,
                                       frame->nb_samples,
                                       audio_enc_ctx->sample_fmt,
                                       0);
    // 1);
    //这里的out nb_samples 必须传输入采样数
//    int convert_nb_samples = swr_convert(swr_ctx, audio_data_buffer, src_frame->nb_samples,
//                                         (const uint8_t **) src_frame->data, src_frame->nb_samples);

    //计算应当编码的样本数
//    int output_frame_size = av_rescale_rnd(swr_get_delay(swr_ctx, 48000) + 1024,
//                                          48000, 44100, AV_ROUND_UP);

    int output_frame_size = 0 ,convert_nb_samples = 0;

    {
        std::lock_guard<std::mutex> lock(swr_mtx);
        output_frame_size = av_rescale_rnd(swr_get_delay(swr_ctx, dst_sample_rate) + frame->nb_samples,
                                               dst_sample_rate, frame->sample_rate, AV_ROUND_UP);


//    int convert_nb_samples = swr_convert(swr_ctx, audio_data_buffer, src_frame->nb_samples,
//                                         (const uint8_t **) src_frame->data, src_frame->nb_samples);
        convert_nb_samples = swr_convert(swr_ctx, audio_data_buffer, output_frame_size,
                                             (const uint8_t **) frame->data, frame->nb_samples);
    }


//    {
//        std::lock_guard<std::mutex> lock(m_mtx);
       // fifo_is_working = true;

    {
        std::lock_guard<std::mutex> lock(fifo_mtx);
        av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + convert_nb_samples);
        av_audio_fifo_write(fifo, (void **) audio_data_buffer, convert_nb_samples);
    }
        //}

//    {
//        std::unique_lock<std::mutex> lock(m_mtx);
//        fifo_is_working = false;
//        cond.notify_all();
//    }



    if (audio_data_buffer) av_freep(&audio_data_buffer[0]);
    av_freep(&audio_data_buffer);


    av_frame_free(&frame);

    {
        std::unique_lock<std::mutex> lock(m_mtx);
        cond.notify_all();
        is_working = false;
    }


    return convert_nb_samples;
}

AVFrame* AudioResampler::get_from_fifo(AVCodecContext *audio_enc_ctx) {
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        while(is_changing){
            cond.wait(lock);
        }
        is_working = true;
    }

//    fifo_is_working = true;
    {
        std::lock_guard<std::mutex> lock(fifo_mtx);
        if ((av_audio_fifo_size(fifo) > 0)) {
            int frame_size = FFMIN(av_audio_fifo_size(fifo), audio_enc_ctx->frame_size);
            AVFrame *dst_frame = av_frame_alloc();
            dst_frame->nb_samples = frame_size;
            dst_frame->channel_layout = audio_enc_ctx->channel_layout;
            dst_frame->format = audio_enc_ctx->sample_fmt;
            dst_frame->sample_rate = audio_enc_ctx->sample_rate;

            av_frame_get_buffer(dst_frame, 0);

//        {
//            std::lock_guard<std::mutex> lock(m_mtx);
            av_audio_fifo_read(fifo, (void **) dst_frame->data, frame_size);
            //}
//        {
//            std::unique_lock<std::mutex> lock(m_mtx);
//            fifo_is_working = false;
//            cond.notify_all();
//        }

            {
                std::unique_lock<std::mutex> lock(m_mtx);
                cond.notify_all();
                is_working = false;
            }

            return dst_frame;
        }
    }

    {
        std::unique_lock<std::mutex> lock(m_mtx);
        cond.notify_all();
        is_working = false;
    }

    return nullptr;
}


void AudioResampler::clear_fifo() {
    std::lock_guard<std::mutex> lock(fifo_mtx);
    av_audio_fifo_reset(fifo);
    av_audio_fifo_free(fifo);
    fifo = nullptr;
}


void AudioResampler::reset_fifo() {
    std::lock_guard<std::mutex> lock(fifo_mtx);
//    int ret = av_audio_fifo_size(fifo);
    av_audio_fifo_reset(fifo);
//    ret = av_audio_fifo_size(fifo);
//    int temp = ret;
}

void AudioResampler::clear_swr() {
    std::lock_guard<std::mutex> lock(swr_mtx);
    swr_free(&swr_ctx);
    swr_ctx = nullptr;
}


int AudioResampler::get_fifo_size() {
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        while(is_changing){
            cond.wait(lock);
        }
        is_working = true;
    }

    int size = 0;
    {
        std::lock_guard<std::mutex> lock(fifo_mtx);
        size = av_audio_fifo_size(fifo);
    }

    {
        std::unique_lock<std::mutex> lock(m_mtx);
        cond.notify_all();
        is_working = false;
    }

    return size;
}


int AudioResampler::reset_resampler(AVCodecContext *audio_dec_ctx, AVCodecContext *audio_enc_ctx) {
    int ret;
    is_changing = true;

    {
        std::unique_lock<std::mutex> lock(m_mtx);
        while(is_working){
            cond.wait(lock);
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_mtx);

        /*
         * 销毁swr_ctx\fifo 清理
         * 初始化新的swr_ctx_fifo
         * */


        reset_fifo();

        //clear_fifo();
        clear_swr();
        //clear_fifo();

        //init_fifo(audio_enc_ctx);

        init_swr(audio_dec_ctx, audio_enc_ctx);

    }


    {
        std::unique_lock<std::mutex> lock(m_mtx);
        cond.notify_all();
        is_changing = false;
    }

    return 0;
}