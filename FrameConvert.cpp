#include "FrameConvert.h"



FrameConverter::FrameConverter(int width, int height, AVPixelFormat pix_fmt,
                               uint64_t channel_layout, int sample_rate, AVSampleFormat sample_format)
                               : width(width),
                                height(height),
                                pixel_fmt(pix_fmt),
                                channel_layout(channel_layout),
                                sample_rate(sample_rate),
                                sample_fmt(sample_format) {

}


FrameConverter::~FrameConverter() {
    swr_free(&swr_ctx);
    swr_ctx = nullptr;

    av_audio_fifo_free(fifo);
    fifo = nullptr;


}


int FrameConverter::init_converter(uint64_t src_channel_layout,
                                   int src_sample_rate,
                                   AVSampleFormat src_sample_fmt) {

    int ret;
    swr_ctx = swr_alloc_set_opts(nullptr,
                                 channel_layout,
                                 sample_fmt,
                                 sample_rate,
                                 src_channel_layout,
                                 src_sample_fmt,
                                 src_sample_rate,
                                 0,
                                 nullptr);

    ret = swr_init(swr_ctx);

    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "swr_ctx init failed.\n");
        return -1;
    }

    fifo = av_audio_fifo_alloc(sample_fmt,
                               av_get_channel_layout_nb_channels(channel_layout),
                               1);
    if (!fifo) {
        av_log(nullptr, AV_LOG_ERROR, "fifo init wrong.\n");
        return -1;
    }


}


int FrameConverter::reset_converter(uint64_t src_channel_layout, int src_sample_rate, AVSampleFormat src_sample_fmt) {
    /*
     * 清理数据 重写swr_ctx, fifo
     *
     * */
    int ret;
    av_audio_fifo_reset(fifo);

    swr_free(&swr_ctx);

    swr_ctx = nullptr;
    swr_ctx = swr_alloc_set_opts(nullptr,
                                 channel_layout,
                                 sample_fmt,
                                 sample_rate,
                                 src_channel_layout,
                                 src_sample_fmt,
                                 src_sample_rate,
                                 0,
                                 nullptr);

    ret = swr_init(swr_ctx);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "swr_ctx init failed.\n");
        return -1;
    }
    return 0;
}


AVFrame* FrameConverter::convert(AVFrame *frame) {
    //video
    if(frame->width > 0){
        int ret;
        int src_width = frame->width;
        int src_height = frame->height;
        AVPixelFormat src_fmt = AV_PIX_FMT_YUV420P;


        AVFrame *dst_frame = av_frame_alloc();
        dst_frame->width =width;
        dst_frame->height = height;
        dst_frame->format = pixel_fmt;
        dst_frame->pts = frame->pts;

        //alloc space for dst frame
        ret= av_frame_get_buffer(dst_frame, 0);
        if(ret < 0){
            av_log(nullptr, AV_LOG_ERROR, "av_frame_get_buffer failed.\n");
            return nullptr;
        }

        SwsContext *sws_ctx = sws_getContext(src_width, src_height, src_fmt,
                                             width, height, pixel_fmt,
                                             SWS_FAST_BILINEAR,
                                             nullptr,
                                             nullptr,
                                             nullptr);
        //修改分辨率、数据帧格式的函数
        ret = sws_scale(sws_ctx, frame->data, frame->linesize, 0, src_height, dst_frame->data, dst_frame->linesize);
        if(ret < 0){
            av_log(nullptr, AV_LOG_ERROR, "sws_scale failed.\n");
            dst_frame = nullptr;
        }

        av_frame_free(&frame);
        frame = nullptr;
        sws_freeContext(sws_ctx);
        return dst_frame;

    } else{ //audio
        uint8_t **audio_data_buffer = nullptr;
        av_samples_alloc_array_and_samples(&audio_data_buffer,
                                           nullptr,
                                           av_get_channel_layout_nb_channels(channel_layout),
                                           frame->nb_samples,
                                           sample_fmt,
                                           0);

        int output_frame_size = 0 ,convert_nb_samples = 0;


        output_frame_size = av_rescale_rnd(swr_get_delay(swr_ctx, sample_rate) + frame->nb_samples,
                                               sample_rate, frame->sample_rate, AV_ROUND_UP);


        convert_nb_samples = swr_convert(swr_ctx, audio_data_buffer, output_frame_size,
                                             (const uint8_t **) frame->data, frame->nb_samples);




        av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + convert_nb_samples);
        av_audio_fifo_write(fifo, (void **) audio_data_buffer, convert_nb_samples);


        if (audio_data_buffer) av_freep(&audio_data_buffer[0]);
        av_freep(&audio_data_buffer);
        av_frame_free(&frame);
        frame = nullptr;



        if ((av_audio_fifo_size(fifo) > 0)) {
            int frame_size = FFMIN(av_audio_fifo_size(fifo), 1024);
            AVFrame *dst_frame = av_frame_alloc();
            dst_frame->nb_samples = frame_size;
            dst_frame->channel_layout = channel_layout;
            dst_frame->format = sample_fmt;
            dst_frame->sample_rate = sample_rate;

            av_frame_get_buffer(dst_frame, 0);

            av_audio_fifo_read(fifo, (void **) dst_frame->data, frame_size);

            return dst_frame;
        }

        return nullptr;
    }
}
