#include "transcoder.h"


std::mutex mtx;
std::condition_variable cond;


std::mutex mtx2;
std::condition_variable cond2;


Transcoder::Transcoder(std::vector<std::string>& input_filenames,
                       std::vector<std::string>& output_filenames,
                       int width,
                       int height,
                       int framerate,
                       int samplerate) :input_filenames(input_filenames),
                                        output_filenames(output_filenames),
                                        width(width),
                                        height(height),
                                        framerate(framerate),
                                        samplerate(samplerate),
                                        inputNums(input_filenames.size()),
                                        outputNums(output_filenames.size()){


}


Transcoder::Transcoder(std::vector<std::string>& output_filenames, int width, int height, int framerate, int samplerate) : output_filenames(output_filenames),
                                                                               width(width),
                                                                               height(height),
                                                                               framerate(framerate),
                                                                               samplerate(samplerate),
                                                                               inputNums(2),
                                                                               outputNums(1){
}



Transcoder::~Transcoder() {
    swr_free(&swr_ctx);
    av_audio_fifo_free(fifo);

    while(!IniterIs.empty()) IniterIs.pop_back();

    while(!decoders.empty()) decoders.pop_back();

    while(!IniterOs.empty()) IniterOs.pop_back();

    while(!encoders.empty()) encoders.pop_back();

}


int Transcoder::init_transcoder() {
    int ret;
    unsigned int i;
    IniterIs.resize(inputNums);
    IniterOs.resize(outputNums);
    decoders.resize(inputNums);
    //encoders.resize(outputNums);
    encoders.resize(1);
    std::vector<AVFormatContext *> ofmt_ctxs(outputNums);

   // packets_over.resize(outputNums, std::vector<int>(2, 0));

    for(i = 0; i < inputNums; i++){
        IniterIs[i] = IniterI::ptr(new IniterI(input_filenames[i]));
        IniterIs[i]->init_fmt();
        decoders[i] = Decoder::ptr(new Decoder(framerate, IniterIs[i]->get_fmt_ctx()));
        decoders[i]->init_decoder();
    }


    encoders[0] = Encoder::ptr(new Encoder(height, width, framerate, samplerate));
    encoders[0]->init_video_encoder();
    encoders[0]->init_audio_encoder();


    int encoder_type;
    bool video_type = true;
   // int first = 1;

    for(i = 0; i < outputNums; i++){
        if(output_filenames[i].substr(0, 3) != "rtp"){
            encoder_type = 0;
        } else{
            encoder_type = video_type ? 1 : 2;
            video_type = (!video_type);
        }

        IniterOs[i] = IniterO::ptr(new IniterO(output_filenames[i]));
        IniterOs[i]->init_fmt();



        if(encoder_type == 0){
            IniterOs[i]->ofmt_create_stream(encoders[0]->get_video_enc_ctx());
            IniterOs[i]->ofmt_create_stream(encoders[0]->get_audio_enc_ctx());
        } else{
            if(encoder_type == 1) IniterOs[i]->ofmt_create_stream(encoders[0]->get_video_enc_ctx());
            else IniterOs[i]->ofmt_create_stream(encoders[0]->get_audio_enc_ctx());
        }


        IniterOs[i]->ofmt_io_open();
        IniterOs[i]->ofmt_print_info();

        ofmt_ctxs[i] = IniterOs[i]->get_fmt_ctx();
//        if(encoder_type != 0){
//            if(encoder_type == 1) packets_over[i][1] = 1;
//            else packets_over[i][0] = 1;
//        }
    }

    distributer = Distributer::ptr(new Distributer(ofmt_ctxs));

    //init swr_ctx and audio_fifo
    swr_ctx = swr_alloc_set_opts(nullptr,
                                 encoders[0]->get_audio_enc_ctx()->channel_layout,
                                 encoders[0]->get_audio_enc_ctx()->sample_fmt,
                                 encoders[0]->get_audio_enc_ctx()->sample_rate,
                                 decoders[0]->get_audio_dec_ctx()->channel_layout,
                                 decoders[0]->get_audio_dec_ctx()->sample_fmt,
                                 decoders[0]->get_audio_dec_ctx()->sample_rate,
                                 0,
                                 nullptr);

    ret = swr_init(swr_ctx);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "swr_ctx init failed.\n");
        return -1;
    }

    fifo = av_audio_fifo_alloc(encoders[0]->get_audio_enc_ctx()->sample_fmt,
                               encoders[0]->get_audio_enc_ctx()->channels, 1);
    if (!fifo) {
        av_log(nullptr, AV_LOG_ERROR, "fifo wrong.\n");
        return -1;
    }


    return 0;
}


int Transcoder::init_local_device_transcoder() {
    int ret;
    unsigned int i;

    //IniterIs.resize(inputNums);
    IniterOs.resize(outputNums);
    decoders.resize(inputNums);
    encoders.resize(outputNums);

    initerD = IniterD::ptr(new IniterD());
    initerD->init_fmt();


//    for(i = 0; i < inputNums; i++){
//        //IniterIs[i] = IniterI::ptr(new IniterI());
//
//        //decoders[i] = Decoder::ptr(new Decoder(framerate, IniterIs[i]->get_fmt_ctx()));
//        decoders[i] = Decoder::ptr(new Decoder(framerate, initerD->get_fmt_ctx()));
//        decoders[i]->init_decoder();
////        decoders[i] = i == 0 ? Decoder::ptr(new Decoder(framerate, initerD->get_vfmt_ctx())) : Decoder::ptr(new Decoder(framerate, initerD->get_afmt_ctx()));
////        decoders[i]->init_decoder();
//
//
//    }

    decoders[0] = Decoder::ptr(new Decoder(framerate, initerD->get_vfmt_ctx()));
    decoders[0]->init_decoder();

//    decoders[1] = Decoder::ptr(new Decoder(framerate, initerD->get_afmt_ctx()));
//    decoders[1]->init_decoder();



    for(i = 0; i < outputNums; i++){
        IniterOs[i] = IniterO::ptr(new IniterO(output_filenames[i]));
        IniterOs[i]->init_fmt();

        encoders[i] = Encoder::ptr(new Encoder(height, width, framerate, samplerate));
        //encoders[i]->init_encoder(IniterOs[i]->get_fmt_ctx());

        IniterOs[i]->ofmt_io_open();
        IniterOs[i]->ofmt_print_info();
    }

    //init swr_ctx and audio_fifo
//    swr_ctx = swr_alloc_set_opts(nullptr,
//                                 encoders[0]->get_audio_enc_ctx()->channel_layout,
//                                 encoders[0]->get_audio_enc_ctx()->sample_fmt,
//                                 encoders[0]->get_audio_enc_ctx()->sample_rate,
//                                 decoders[1]->get_audio_dec_ctx()->channel_layout,
//                                 decoders[1]->get_audio_dec_ctx()->sample_fmt,
//                                 decoders[1]->get_audio_dec_ctx()->sample_rate,
//                                 0,
//                                 nullptr);
//
//    ret = swr_init(swr_ctx);
//    if(ret < 0){
//        av_log(nullptr, AV_LOG_ERROR, "swr_ctx init failed.\n");
//        return -1;
//    }
//
//    fifo = av_audio_fifo_alloc(encoders[0]->get_audio_enc_ctx()->sample_fmt, encoders[0]->get_audio_enc_ctx()->channels, 1);
//    if(!fifo){
//        av_log(nullptr, AV_LOG_ERROR, "fifo wrong.\n");
//        return -1;
//    }

    return 0;

}



int Transcoder::reencode() {
    int ret;
    unsigned int i;



    std::vector<int> works(inputNums, 1);


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


    //开启视频处理线程
    Thread thread_video(&Transcoder::dealing_video, this, std::ref(works));



    //开启音频处理线程
    Thread thread_audio(&Transcoder::dealing_audio, this, std::ref(works));


    while(true){
        if(video_packet_over == 1 && audio_packet_over == 1){
            for(i = 0; i < outputNums; i++){
                Writer::write_tail(IniterOs[i]->get_fmt_ctx());
            }
            break;
        }
    }


    return 0;
}



int Transcoder::reencode_local_device() {
    int ret;

    Writer::write_header(IniterOs[0]->get_fmt_ctx());
    //Write--->packets

    if(output_filenames[0].substr(0, 4) == "rtmp"){
        // ret = encode_files_as_rtmp();
        //ret = encode_files_rtmp_threads();
        // ret = device2mp4();
        //ret = device2rtmp();
    }
    if(output_filenames[0].substr(output_filenames[0].size() - 4, 4) == ".mp4"){
        //ret = encode_files_as_mp4();
        //ret = encode_files_mp4_threads();
       // ret = device2mp4();
    }


    Writer::write_tail(IniterOs[0]->get_fmt_ctx());
    return 0;

}




int Transcoder::dealing_video(std::vector<int>& works){
    unsigned int i;
    bool video_finish;
    int64_t last_time = -1;

    std::vector<AVFrame *> last_frames(inputNums);
    std::vector<AVFrame *> merge_frames(inputNums);
    for(int i = 0; i < last_frames.size(); i++){
        last_frames[i] = FrameCreater::create_video_frame();
        merge_frames[i] = FrameCreater::create_video_frame();
    }

    while(true){
        video_finish = true;
        for(i = 0; i < inputNums; i++){
            video_finish = video_finish && (decoders[i]->get_video_queue().empty()) && (works[i] == 0);
        }
        if(video_finish) break;


        for(i = 0; i < inputNums; i++){
            if(!decoders[i]->get_video_queue().empty()){
                av_frame_free(&last_frames[i]);
                last_frames[i] = av_frame_clone(decoders[i]->get_video_queue_front());

                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(decoders[i]->get_video_queue_front());
                decoders[i]->pop_video();

            } else{
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(last_frames[i]);
            }
        }

        encoders[0]->encode_video(distributer, Utils::merge_way(width, height, merge_frames), last_time);

        //encoders[0]->encode_video()
        //encoders[encode_index]->encode_video_rtsp(Utils::merge_way(width, height, merge_frames), last_time);

        {
            std::unique_lock<std::mutex> lock(mtx);
            cond.notify_all();
        }
    }

    encoders[0]->encode_video(distributer, nullptr, last_time);


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
    //packets_over[0][0] = 1;
    return 0;
}



int Transcoder::dealing_audio(std::vector<int>& works){
        int ret;
    unsigned int i;

    int64_t last_time = -1;
    bool audio_finish;
    bool all_audio_finish;


    while(true){
        //判断是否所有音频帧结束
        audio_finish = true;
        for(i = 0; i < inputNums; i++){
            audio_finish = audio_finish && (decoders[i]->get_audio_queue().empty()) && (works[i] == 0);
        }
        if(audio_finish && av_audio_fifo_size(fifo) <= 0) break;


        //使用计数是为了保证fifo中存足够的音频数据
        int time = 2;
        while(time--){
            for(i = 0; i < inputNums; i++){
                if(i == 0){
                    if(!decoders[i]->get_audio_queue().empty()) {
                        Utils::write_to_fifo(fifo, swr_ctx,
                                             av_frame_clone(decoders[i]->get_audio_queue_front()),
                                             encoders[0]->get_audio_enc_ctx()->channels,
                                             encoders[0]->get_audio_enc_ctx()->sample_fmt,
                                             encoders[0]->get_audio_enc_ctx()->sample_rate);

                        decoders[i]->pop_audio();
                    }
                } else{
                    //因为暂时只用到了第一个视频的音频 所以其余音频帧需要被释放
                    while(!decoders[i]->get_audio_queue().empty()){
                        decoders[i]->pop_audio();
                    }
                }
            }
        }

        all_audio_finish = works[0] == 0 && decoders[0]->get_audio_queue().empty();

        if(av_audio_fifo_size(fifo) >= encoders[0]->get_audio_enc_ctx()->frame_size || (av_audio_fifo_size(fifo) > 0 && all_audio_finish)){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
            if(audio_frame){
                ret = encoders[0]->encode_audio(distributer, audio_frame, last_time);
                if(ret < 0) return -1;
            } else break;
        }

    }
    encoders[0]->encode_audio(distributer, nullptr, last_time);

    audio_packet_over = 1;
    //packets_over[0][1] = 1;
    return 0;
}
