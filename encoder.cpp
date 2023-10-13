#include "encoder.h"

#include <mutex>

extern std::mutex mtx2;

Encoder::Encoder(int height, int width, int framerate, int samplerate) : height(height),
                                                                width(width),
                                                                framerate(framerate),
                                                                samplerate(samplerate){
}


Encoder::~Encoder() {
    avcodec_free_context(&audio_enc_ctx);
    avcodec_free_context(&video_enc_ctx);
}


int Encoder::init_video_encoder(AVFormatContext *ofmt_ctx) {
    int ret;

    AVCodecContext *enc_ctx;
    const AVCodec *encoder = avcodec_find_encoder_by_name("libx264");
    //const AVCodec *encoder = avcodec_find_encoder_by_name("h264_nvenc");

    if (!encoder) {
        av_log(nullptr, AV_LOG_FATAL, "Necessary encoder not found\n");
        return AVERROR_INVALIDDATA;
    }

    enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx) {
        av_log(nullptr, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
        return AVERROR(ENOMEM);
    }

    enc_ctx->height = height;
    enc_ctx->width = width;

    //硬编参数

    enc_ctx->max_b_frames = 0;
    enc_ctx->gop_size = 1;

    //硬编码参数设置
//    enc_ctx->bit_rate = 800000;
//    enc_ctx->bit_rate_tolerance = 100000;
//    enc_ctx->rc_min_rate = 650000;
//    enc_ctx->rc_max_rate = 850000;
//    enc_ctx->rc_buffer_size = (int)enc_ctx->bit_rate;
//    enc_ctx->rc_initial_buffer_occupancy = enc_ctx->rc_buffer_size * 3 / 4;


    /* take first format from list of supported formats */
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    /* video time_base can be set to whatever is handy and supported by encoder */
    enc_ctx->framerate = AVRational{framerate, 1};
    enc_ctx->time_base = av_inv_q(enc_ctx->framerate); // {1, framerate}
    video_enc_ctx = enc_ctx;


//  "veryfast": 比 "superfast" 稍慢一些，提供更好的画质。
//  "faster": 比 "veryfast" 稍慢一些，进一步提升画质。
//  "fast": 比 "faster" 稍慢一些，综合了速度和画质。
//  "medium": 中等速度和画质的预设，提供了较好的平衡。
//   "slow": 比 "medium" 慢一些，提供更高的画质。
//  "slower": 比 "slow" 更慢，进一步提升画质。
//  "veryslow": 最慢的预设，提供最高质量的编码结果


    av_opt_set(video_enc_ctx->priv_data, "preset", "ultrafast", 0);
// av_opt_set(video_enc_ctx->priv_data, "tune", "zerolatency", 0);
// av_opt_set(video_enc_ctx->priv_data, "profile", "main", 0);

    time_per_frame_video = 1.0 * 1000 / framerate;

    //enc_ctx->flags = -2143289344;

    if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


    ret = avcodec_open2(enc_ctx, encoder, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", 0);
        return ret;
    }

    return 0;
}


int Encoder::init_audio_encoder(AVFormatContext *ofmt_ctx) {
    int ret;

    AVCodecContext *enc_ctx;
    const AVCodec *encoder = avcodec_find_encoder(AV_CODEC_ID_AAC);

    if (!encoder) {
        av_log(nullptr, AV_LOG_FATAL, "Necessary encoder not found\n");
        return AVERROR_INVALIDDATA;
    }

    enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx) {
        av_log(nullptr, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
        return AVERROR(ENOMEM);
    }

    enc_ctx->sample_rate = samplerate;
    enc_ctx->channel_layout = 3;
    enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);

    /* take first format from list of supported formats */
    enc_ctx->sample_fmt = encoder->sample_fmts[0];
    enc_ctx->time_base = AVRational{1, enc_ctx->sample_rate};
    audio_enc_ctx = enc_ctx;

    time_per_frame_audio = 1024 * 1.0 * 1000 / audio_enc_ctx->sample_rate;

    //enc_ctx->flags = 4194304;
    if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    ret = avcodec_open2(enc_ctx, encoder, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", 1);
        return ret;
    }

    return 0;
}




int Encoder::encode_video(Distributer::ptr distributer, AVFrame *frame, int64_t &last_time) {
    int ret;
    int stream_index = 0;
    AVFrame *encode_frame = frame;
    AVPacket *enc_pkt = av_packet_alloc();

    if (encode_frame) encode_frame->pts = (int64_t)video_pts;
    video_pts++;

    ret = avcodec_send_frame(video_enc_ctx, encode_frame);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "video_frame: avcodec_send_frame failed.\n");
        return ret;
    }

    av_frame_free(&frame);
    while (ret >= 0) {
        ret = avcodec_receive_packet(video_enc_ctx, enc_pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;

        enc_pkt->stream_index = stream_index;

        enc_pkt->dts = enc_pkt->pts = current_time_video;
        current_time_video += time_per_frame_video;

//        av_log(nullptr, AV_LOG_INFO, "pts: #%d ", enc_pkt->pts);
//        av_log(nullptr, AV_LOG_INFO, " dts: #%d ", enc_pkt->dts);
//        av_log(nullptr, AV_LOG_INFO, "video packet.\n");

        //enc_pkt->pts = enc_pkt->dts = (enc_pkt->pts * ofmt_ctx->streams[stream_index]->time_base.den / ofmt_ctx->streams[stream_index]->time_base.num / 1000);


        if(last_time != -1){
            while(1000 / framerate * 1000 - (Timer::getCurrentTime() - last_time) > 1000){
                av_usleep(100);
            }
        }


        distributer->distribute(enc_pkt);
        // distributer->distribute(av_packet_clone(enc_pkt));

        //Writer::write_packets(ofmt_ctx, enc_pkt);

        av_log(nullptr, AV_LOG_INFO, "encode one video packet: %dms.\n", (Timer::getCurrentTime() - last_time) / 1000);
        last_time = Timer::getCurrentTime();
    }

    av_packet_free(&enc_pkt);
    return 0;
}



int Encoder::encode_audio(Distributer::ptr distributer, AVFrame *frame, int64_t &last_time) {
    int64_t func_time = Timer::getCurrentTime();
    int ret;
    int stream_index = 1;
    AVFrame *encode_frame = frame;
    AVPacket *enc_pkt = av_packet_alloc();


    if(encode_frame) encode_frame->pts = audio_pts;
    audio_pts += 1024;

    ret = avcodec_send_frame(audio_enc_ctx, encode_frame);
    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "audio_frame: avcodec_send_frame failed.\n");
        return ret;
    }

    av_frame_free(&encode_frame);
    while (ret >= 0) {
        ret = avcodec_receive_packet(audio_enc_ctx, enc_pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;


        enc_pkt->stream_index = stream_index;

        enc_pkt->dts = enc_pkt->pts = current_time_audio;
        current_time_audio += time_per_frame_audio;

//        av_log(nullptr, AV_LOG_INFO, "pts: #%d ", enc_pkt->pts);
//        av_log(nullptr, AV_LOG_INFO, " dts: #%d ", enc_pkt->dts);
//        av_log(nullptr, AV_LOG_INFO, "audio packet.\n");

        //enc_pkt->pts = enc_pkt->dts = (enc_pkt->pts * ofmt_ctx->streams[stream_index]->time_base.den / ofmt_ctx->streams[stream_index]->time_base.num / 1000);

        if(last_time != -1){
            while(audio_enc_ctx->frame_size * 1000 / audio_enc_ctx->sample_rate * 1000 - (Timer::getCurrentTime() - last_time) > 500){
                av_usleep(100);
            }
        }


        distributer->distribute(enc_pkt);

        //distributer->distribute(av_packet_clone(enc_pkt));
       // Writer::write_packets(ofmt_ctx, enc_pkt);

        av_log(nullptr, AV_LOG_INFO, "encode one audio packet: %dms.\n", (Timer::getCurrentTime() - last_time) / 1000);
        last_time = Timer::getCurrentTime();
    }

    av_packet_free(&enc_pkt);
    return 0;
}

