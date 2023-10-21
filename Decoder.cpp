#include "Decoder.h"
#include <iostream>

Decoder::Decoder(int set_framerate, AVFormatContext *ifmt_ctx, FrameConverter::ptr converter) :
                                                                    ifmt_ctx(ifmt_ctx),
                                                                    set_framerate(set_framerate),
                                                                    converter(converter){

}



Decoder::~Decoder() {
    ifmt_ctx = nullptr;

    clear_video();
    clear_audio();

    if(audio_dec_ctx) avcodec_free_context(&audio_dec_ctx);
    if(video_dec_ctx) avcodec_free_context(&video_dec_ctx);
}

int Decoder::init_decoder() {
    int ret;
    unsigned int i;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *stream = ifmt_ctx->streams[i];
        const AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
        AVCodecContext *codec_ctx;
        if (!dec) {
            av_log(nullptr, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
            return AVERROR_DECODER_NOT_FOUND;
        }

        codec_ctx = avcodec_alloc_context3(dec);
        if (!codec_ctx) {
            av_log(nullptr, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
            return AVERROR(ENOMEM);
        }
        ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
                                          "for stream #%u\n", i);
            return ret;
        }

        /* Reencode video & audio and remux subtitles etc. */
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                codec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, stream, nullptr);
                codec_ctx->bit_rate = ifmt_ctx->bit_rate; //new
            }

            ret = avcodec_open2(codec_ctx, dec, nullptr);
            if (ret < 0) {
                av_log(nullptr, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
                return ret;
            }
        }


        if(codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO){
            video_dec_ctx = codec_ctx;
            real_framerate = (video_dec_ctx->framerate.num * 1.0 / video_dec_ctx->framerate.den);
        }
        else {
            audio_dec_ctx = codec_ctx;
            audio_dec_ctx->channel_layout = av_get_default_channel_layout(audio_dec_ctx->channels);
            if(ifmt_ctx->nb_streams == 1) real_framerate = set_framerate;
        }
    }
    return 0;
}



int Decoder::decode(int& work) {
    int ret;

    while(true) {
        //实际帧率大于设置的帧率，需要丢帧
        if (real_framerate >= set_framerate) {
            frameratio = (real_framerate - set_framerate) * 1.0 / set_framerate;
            ret = decode_high2low();
            if (ret < 0) {
                av_log(nullptr, AV_LOG_ERROR, "Decoder::decode_high2low() failed.\n");
            }
            if(ret == 0) break;
            if(ret == 1) continue;
            //实际帧率小于设置的帧率，需要补帧
        } else {
            frameratio = (set_framerate - real_framerate) * 1.0 / real_framerate;
            ret = decode_low2high();
            if (ret < 0) {
                av_log(nullptr, AV_LOG_ERROR, "Decoder::decode_low2high() failed.\n");
            }
            if(ret == 0) break;
            if(ret == 1) continue;
        }
    }
    work = 0;
    return ret;
}

int Decoder::decode_high2low() {
    int ret;
    AVPacket *packet = av_packet_alloc();
    int time = 0;
    while(true) {

        if(is_changing){
            av_packet_free(&packet);
            return 1;
        }
        std::unique_lock<std::mutex> lock(m_mtx);
        if ((ret = av_read_frame(ifmt_ctx, packet)) < 0) {
            if (time > 0) {
                time--;
                av_seek_frame(ifmt_ctx, -1, 0, AVSEEK_FLAG_FRAME);
                avformat_seek_file(ifmt_ctx, -1, 0, 0, INT_MAX, 0);
                avcodec_flush_buffers(video_dec_ctx);
                avcodec_flush_buffers(audio_dec_ctx);
                continue;
            } else {
                break;
            }
        }

        int stream_index = packet->stream_index;
        if (stream_index == 0) ret = avcodec_send_packet(video_dec_ctx, packet);
        else ret = avcodec_send_packet(audio_dec_ctx, packet);

        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "Decoding failed id:%d.\n", ret);
            break;
        }

        av_packet_unref(packet);
        AVFrame *dec_frame = av_frame_alloc();
        while (ret >= 0) {
            if (stream_index == 0) ret = avcodec_receive_frame(video_dec_ctx, dec_frame);
            else ret = avcodec_receive_frame(audio_dec_ctx, dec_frame);

            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) break;
            else if (ret < 0) return ret;
            dec_frame->pts = dec_frame->best_effort_timestamp;
            dec_frame->pict_type = AV_PICTURE_TYPE_NONE;
            //如果是视频帧 需要判断丢帧
            if (stream_index == 0) {
                if (count >= 1.0f) {
                    while (count >= 1.0f) {
                        count -= 1.0f;
                    }
                } else {
                    count += frameratio;

                    {
                        while (video_queue.size() >= video_queue_cache) {
                            av_usleep(100);
                        }
                    }

                    video_queue.push_back(converter->convert(av_frame_clone(dec_frame)));
                }
            } else {
                if(using_audio) {
                    AVFrame *temp_audio = converter->convert(av_frame_clone(dec_frame));
                    if (temp_audio) audio_queue.push_back(temp_audio);
                }
            }
            av_frame_unref(dec_frame);
        }
        av_frame_free(&dec_frame);
    }
    av_packet_free(&packet);
    return 0;
}

int Decoder::decode_low2high() {
    AVPacket *packet = av_packet_alloc();
    int ret;
    int time = 0;
    while(true) {
        if(is_changing){
            av_packet_free(&packet);
            return 1;
        }
        std::unique_lock<std::mutex> lock(m_mtx);
        if (count < 1.0f) {
            if ((ret = av_read_frame(ifmt_ctx, packet)) < 0) {
                if (time > 0) {
                    time--;
                    av_seek_frame(ifmt_ctx, -1, 0, AVSEEK_FLAG_FRAME);
                    avformat_seek_file(ifmt_ctx, -1, 0, 0, INT_MAX, 0);
                    avcodec_flush_buffers(video_dec_ctx);
                    avcodec_flush_buffers(audio_dec_ctx);
                    continue;
                } else {
                    break;
                }
            }


            int stream_index = packet->stream_index;

            if (stream_index == 0) ret = avcodec_send_packet(video_dec_ctx, packet);
            else ret = avcodec_send_packet(audio_dec_ctx, packet);

            if (ret < 0) {
                av_log(nullptr, AV_LOG_ERROR, "Decoding failed.\n");
                break;
            }


            av_packet_unref(packet);
            AVFrame *dec_frame = av_frame_alloc();
            while (ret >= 0) {

                if (stream_index == 0) ret = avcodec_receive_frame(video_dec_ctx, dec_frame);
                else ret = avcodec_receive_frame(audio_dec_ctx, dec_frame);

                if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) break;
                else if (ret < 0) return ret;


                dec_frame->pts = dec_frame->best_effort_timestamp;
                dec_frame->pict_type = AV_PICTURE_TYPE_NONE;
                if (stream_index == 0) {
                    //写数据加锁

                    while (video_queue.size() >= video_queue_cache) {
                        av_usleep(1000);
                    }

                    video_queue.push_back(converter->convert(av_frame_clone(dec_frame)));
                    if(count + frameratio >= 1.0f) prev_frame = av_frame_clone(dec_frame);
                    count += frameratio;
                } else {
                    if(using_audio){
                        AVFrame *temp_audio = converter->convert(av_frame_clone(dec_frame));
                        if(temp_audio) audio_queue.push_back(temp_audio);
                    }
                }
                av_frame_unref(dec_frame);
            }
            av_frame_free(&dec_frame);
        } else {
            while (count >= 1.0f) {
                count -= 1.0f;
                while (video_queue.size() >= video_queue_cache) {
                    av_usleep(1000);
                }

                video_queue.push_back(converter->convert(av_frame_clone(prev_frame)));
                if(count < 1.0f){
                    av_frame_unref(prev_frame);
                    av_frame_free(&prev_frame);
                }
            }
        }
    }
    av_packet_free(&packet);
    return 0;
}




int Decoder::change_fmt(AVFormatContext *fmt_ctx) {
    int64_t func_time = Timer::getCurrentTime();
    is_changing = true;
    std::unique_lock<std::mutex> lock(m_mtx);
    int ret;

    clear_audio();
    clear_video();
    avformat_close_input(&ifmt_ctx);
    ifmt_ctx = nullptr;
    ifmt_ctx = fmt_ctx;

    if (audio_dec_ctx) avcodec_free_context(&audio_dec_ctx);
    if (video_dec_ctx) avcodec_free_context(&video_dec_ctx);
    ret = init_decoder();
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Decoder::change_fmt() failed.\n");
        return ret;
    }
    if(using_audio) converter->reset_converter(get_audio_dec_ctx()->channel_layout, get_audio_dec_ctx()->sample_rate, get_audio_dec_ctx()->sample_fmt);

    av_log(nullptr, AV_LOG_INFO, "Decoder::change_fmt() using time: %dms.\n", (Timer::getCurrentTime() - func_time) / 1000);

    is_changing = false;

    return 0;
}


AVCodecContext* Decoder::get_video_dec_ctx() const {
    return video_dec_ctx;
}


AVCodecContext* Decoder::get_audio_dec_ctx() const {
    return audio_dec_ctx;
}


void Decoder::use_audio() {
    using_audio = true;
}

bool Decoder::is_using_audio() {
    return using_audio;
}


void Decoder::pop_audio_front() {
    //std::lock_guard<std::mutex> lock(a_mtx);
    if(!is_audio_empty()) audio_queue.pop_front();
}

void Decoder::pop_video_front() {
    //std::lock_guard<std::mutex> lock(v_mtx);
    if(!is_video_empty()) video_queue.pop_front();
}

void Decoder::pop_audio_back() {
    //std::lock_guard<std::mutex> lock(a_mtx);
    if(!is_audio_empty()) audio_queue.pop_back();
}


void Decoder::pop_video_back() {
    //std::lock_guard<std::mutex> lock(v_mtx);
    if(!is_video_empty()) video_queue.pop_back();
}




AVFrame *Decoder::get_audio_front(){
    //std::lock_guard<std::mutex> lock(a_mtx);
    return is_audio_empty() ? nullptr : audio_queue.front();
}


AVFrame *Decoder::get_video_front(){
    //std::lock_guard<std::mutex> lock(v_mtx);
    return is_video_empty() ? nullptr : video_queue.front();
}



bool Decoder::is_audio_empty() {
    //std::lock_guard<std::mutex> lock(a_mtx);
    return audio_queue.empty();
}

bool Decoder::is_video_empty() {
    //std::lock_guard<std::mutex> lock(v_mtx);
    return video_queue.empty();
}



AVFrame* Decoder::get_audio() {
    std::lock_guard<std::mutex> lock(a_mtx);
    if(!audio_queue.empty()){
        AVFrame* frame = av_frame_clone(audio_queue.front());
        audio_queue.pop_front();
        return frame;
    }
    return nullptr;
}


AVFrame* Decoder::get_video() {
    std::lock_guard<std::mutex> lock(v_mtx);
    if(!video_queue.empty()){
        AVFrame* frame = av_frame_clone(video_queue.front());
        video_queue.pop_front();
        return frame;
    }
    return nullptr;
}




void Decoder::clear_audio() {
    std::lock_guard<std::mutex> lock(a_mtx);
    while(!audio_queue.empty()){
        audio_queue.pop_back();
    }
}



void Decoder::clear_video() {
    std::lock_guard<std::mutex> lock(v_mtx);
    while(!video_queue.empty()){
       video_queue.pop_back();
    }
}

