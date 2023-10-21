#include "Transcoder.h"


Transcoder::Transcoder(std::vector<std::string> &input_filenames,
                       std::vector<std::string> &output_filenames,
                       int width,
                       int height,
                       AVPixelFormat pix_fmt,
                       int framerate,
                       uint64_t channel_layout,
                       int samplerate,
                       AVSampleFormat sample_fmt) : input_filenames(input_filenames),
                                                    output_filenames(output_filenames),
                                                    width(width),
                                                    height(height),
                                                    pix_fmt(pix_fmt),
                                                    framerate(framerate),
                                                    channel_layout(channel_layout),
                                                    samplerate(samplerate),
                                                    sample_fmt(sample_fmt),
                                                    inputNums(input_filenames.size()),
                                                    outputNums(output_filenames.size()){}





Transcoder::~Transcoder() {
    while(!IniterIs.empty()) IniterIs.pop_back();

    while(!decoders.empty()) decoders.pop_back();

    while(!IniterOs.empty()) IniterOs.pop_back();

    while(!encoders.empty()) encoders.pop_back();

}



int Transcoder::init_INiterIs() {
    int ret;
    unsigned int i;
    for(i = 0; i < inputNums; i++){
        IniterIs[i] = IniterI::ptr(new IniterI(input_filenames[i]));
        ret = IniterIs[i]->init_fmt();
        if(ret < 0){
            av_log(nullptr, AV_LOG_ERROR, "init Initers[%d] failed.\n", i);
        }
    }

    return ret;
}


int Transcoder::init_Decoders() {
    int ret;
    unsigned int i;
    for(i = 0; i < inputNums; i++){
        decoders[i] = Decoder::ptr(new Decoder(framerate, IniterIs[i]->get_fmt_ctx(), converter));
        ret = decoders[i]->init_decoder();
        if(ret < 0){
            av_log(nullptr, AV_LOG_ERROR, "init decoders[%d] failed.\n", i);
        }
        if(i == 0){
            decoders[i]->use_audio();
            converter->init_converter(decoders[i]->get_audio_dec_ctx()->channel_layout,
                                      decoders[i]->get_audio_dec_ctx()->sample_rate,
                                      decoders[i]->get_audio_dec_ctx()->sample_fmt);
        }
    }
    return ret;
}


int Transcoder::init_IniterOs(bool& initerO_has_inited, bool& encoder_has_inited, std::vector<AVFormatContext *>& ofmt_ctxs) {
    int ret;
    unsigned int i;

    for(i = 0; i < outputNums; i++){
        IniterOs[i] = IniterO::ptr(new IniterO(output_filenames[i]));
        ret = IniterOs[i]->init_fmt();
        if(ret < 0){
            av_log(nullptr, AV_LOG_ERROR, "interOs[%d] failed.\n");
            return -1;
        }
    }

    initerO_has_inited = true;

    while(!encoder_has_inited) continue;


    int encoder_type;
    bool video_type = true;
    for(i = 0; i < outputNums; i++){
        if(std::string(IniterOs[i]->get_fmt_ctx()->oformat->name) != "rtp"){
            encoder_type = 0;
        } else{
            encoder_type = video_type ? 1 : 2;
            video_type = (!video_type);
        }


        if(encoder_type == 0){
            IniterOs[i]->ofmt_create_stream(encoders[0]->get_video_enc_ctx());
            IniterOs[i]->ofmt_create_stream(encoders[0]->get_audio_enc_ctx());
        } else{
            if(encoder_type == 1) IniterOs[i]->ofmt_create_stream(encoders[0]->get_video_enc_ctx());
            else IniterOs[i]->ofmt_create_stream(encoders[0]->get_audio_enc_ctx());
        }


        IniterOs[i]->ofmt_io_open();
        IniterOs[i]->ofmt_print_info();

        ofmt_ctxs.push_back(IniterOs[i]->get_fmt_ctx());
    }


    return 0;
}



int Transcoder::init_Encoder(bool& initerO_has_inited, bool& encoder_has_inited) {
    int ret;
    unsigned int i;


    encoders[0] = Encoder::ptr(new Encoder(width, height, pix_fmt ,framerate, channel_layout, sample_fmt, samplerate));
    while(!initerO_has_inited) continue;

    for(i = 0; i < inputNums; i++){
        if(std::string(IniterOs[i]->get_fmt_ctx()->oformat->name) == "rtp") continue;
        else break;
    }

    if(i >= inputNums) i = 0;
    ret = encoders[0]->init_video_encoder(IniterOs[i]->get_fmt_ctx());
    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "Encoder::init_video_encoder() failed.\n");
        return ret;
    }

    ret = encoders[0]->init_audio_encoder(IniterOs[i]->get_fmt_ctx());
    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "Encoder::init_audio_encoder() failed.\n");
        return ret;
    }

    encoder_has_inited = true;
    return 0;
}



int Transcoder::init_transcoder() {
    int64_t func_time = Timer::getCurrentTime();
    int ret;
    unsigned int i;
    IniterIs.resize(inputNums);
    IniterOs.resize(outputNums);
    decoders.resize(inputNums);
    encoders.resize(1);

    converter = FrameConverter::ptr(new FrameConverter(width, height, pix_fmt, channel_layout, samplerate, sample_fmt));


    bool encoder_has_inited = false, initerO_has_inited = false;

    std::vector<AVFormatContext *> ofmt_ctxs = {};

    Thread thread_init_IniterOs(&Transcoder::init_IniterOs, this, std::ref(initerO_has_inited), std::ref(encoder_has_inited),std::ref(ofmt_ctxs));

    Thread thread_init_Encoder(&Transcoder::init_Encoder, this, std::ref(initerO_has_inited), std::ref(encoder_has_inited));

    ret = init_INiterIs();
    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "Transcoder::init_INiterIs() failed.\n");
        return ret;
    }
    ret = init_Decoders();
    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "Transcoder::init_Decoders() failed.\n");
        return ret;
    }

    while(ofmt_ctxs.size() != outputNums) continue;
    distributer = Distributer::ptr(new Distributer(ofmt_ctxs));


    av_log(nullptr, AV_LOG_INFO, "whole init time: %dms.\n", (Timer::getCurrentTime() - func_time) / 1000);
    return 0;

}



int Transcoder::transcode() {
    int64_t func_time = Timer::getCurrentTime();
    unsigned int i;


    works.resize(inputNums, 1);


    /*
     * 修改逻辑 在这里直接开启解码线程、音视频编码线程（统一的方法）
     *
     * */
    for(i = 0; i < inputNums; i++){
        Thread thread_decode(&Decoder::decode, decoders[i], std::ref(works[i]));
        thread_decode.detach();
    }

    for(i = 0; i < outputNums; i++){
        Writer::write_header(IniterOs[i]->get_fmt_ctx());
    }


    distributer->start();

    /*
     *  分别开启以下工作线程
     *  写fifo
     *  清理不需要的音频
     *  写视频包
     *  写音频包/读fifo
     *
     * */

    Thread thread_video(&Transcoder::dealing_video, this, std::ref(works));

    Thread thread_audio(&Transcoder::dealing_audio, this, std::ref(works));


    av_log(nullptr, AV_LOG_INFO, "main threads init time: %dms.\n", (Timer::getCurrentTime() - func_time) / 1000 );


    while(video_packet_over != 1 || audio_packet_over != 1){
        av_usleep(1000);
    }

    distributer->stop();
    for(i = 0; i < outputNums; i++){
        Writer::write_tail(IniterOs[i]->get_fmt_ctx());
    }

    return 0;
}



int Transcoder::dealing_audio(std::vector<int> &works) {
    int ret;
    bool audio_finish;

    while(true){
        audio_finish = works[0] == 0 && decoders[0]->is_audio_empty();
        if(audio_finish) break;

        if (!decoders[0]->is_audio_empty()) {
            AVFrame *temp_frame = decoders[0]->get_audio();
            if(temp_frame){
                ret = encoders[0]->encode_audio(distributer, temp_frame);
                if(ret < 0){
                    av_log(nullptr, AV_LOG_ERROR, "encoders[0]->encode_audio() failed.\n");
                    return -1;
                }
            }
        }
    }
    encoders[0]->encode_audio(distributer, nullptr);
    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "encoders[0]->encode_audio() failed.\n");
        return -1;
    }

    audio_packet_over = 1;
    return 0;

}



int Transcoder::dealing_video(std::vector<int>& works){
    int ret;
    unsigned int i;
    bool video_finish;

    std::vector<AVFrame *> last_frames(inputNums);
    std::vector<AVFrame *> merge_frames(inputNums);
    for(int i = 0; i < last_frames.size(); i++){
        last_frames[i] = FrameCreater::create_video_frame();
        merge_frames[i] = FrameCreater::create_video_frame();
    }

    while(true){
        video_finish = true;
        for(i = 0; i < inputNums; i++){
            video_finish = video_finish && (decoders[i]->is_video_empty()) && works[i] == 0;
        }
        if(video_finish) break;

        for (i = 0; i < inputNums; i++) {
            if (!decoders[i]->is_video_empty()) {
                AVFrame * temp_frame = decoders[i]->get_video();
                if(temp_frame) {
                    av_frame_free(&last_frames[i]);
                    last_frames[i] = temp_frame;
                    av_frame_free(&merge_frames[i]);
                    merge_frames[i] = av_frame_clone(last_frames[i]);
                } else{
                    av_frame_free(&merge_frames[i]);
                    merge_frames[i] = av_frame_clone(last_frames[i]);
                }
            } else {
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(last_frames[i]);
            }
        }
        ret = encoders[0]->encode_video(distributer, FrameConverter::merge_frames(width, height, merge_frames));
        if(ret < 0){
            av_log(nullptr, AV_LOG_ERROR, "encoders[0]->encode_video() failed.\n");
            return -1;
        }
    }

    ret = encoders[0]->encode_video(distributer, nullptr);
    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "encoders[0]->encode_video() failed.\n");
        return -1;
    }

    for(int i = 0; i < inputNums; i++){
        av_frame_free(&last_frames[i]);
        av_frame_free(&merge_frames[i]);
    }
    while(!last_frames.empty()){
        last_frames.pop_back();
        merge_frames.pop_back();
    }
    av_usleep(7000000);


    video_packet_over = 1;
    return 0;
}



int Transcoder::change_input_stream(std::string &filename, int& stream_index) {
    if(works.empty()){
        av_log(nullptr, AV_LOG_ERROR, "vector-works now is empty.\n");
        return 0;
    }

    if(video_packet_over == 1){
        av_log(nullptr, AV_LOG_ERROR, "the transcode program is close to finish, permission denied.\n");
        return 0;
    }

    decoders[stream_index]->change_fmt(IniterIs[stream_index]->change_fmt(filename));

    //某路流已经结束
    if(works[stream_index] == 0){
        works[stream_index] = 1;

        Thread thread_deocde(&Decoder::decode, decoders[stream_index], std::ref(works[stream_index]));
        thread_deocde.detach();

        if(stream_index == 0) {
            decoders[stream_index]->use_audio();
            audio_packet_over = 0;
            Thread dealing_audio(&Transcoder::dealing_audio, this, std::ref(works));
        }
    }

    return 0;
}





