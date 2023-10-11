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

   while(IniterIs.size() > 0 ){
     //  decoders.pop_back();
       IniterIs.pop_back();
   }

   while(decoders.size() > 0){
       decoders.pop_back();
   }


   while(IniterOs.size() > 0){
       IniterOs.pop_back();
   }

   while(encoders.size() > 0){
       encoders.pop_back();
   }

//   while(outputNums--){
//       IniterOs.pop_back();
//       encoders.pop_back();
//   }
}


int Transcoder::init_transcoder() {
    int ret;
    unsigned int i;
    IniterIs.resize(inputNums);
    IniterOs.resize(outputNums);
    decoders.resize(inputNums);
    encoders.resize(outputNums);
    for(i = 0; i < inputNums; i++){
        IniterIs[i] = IniterI::ptr(new IniterI(input_filenames[i]));
        IniterIs[i]->init_fmt();
        decoders[i] = Decoder::ptr(new Decoder(framerate, IniterIs[i]->get_fmt_ctx()));
        decoders[i]->init_decoder();
    }

    for(i = 0; i < outputNums; i++){
        IniterOs[i] = IniterO::ptr(new IniterO(output_filenames[i]));
        IniterOs[i]->init_fmt();

        encoders[i] = Encoder::ptr(new Encoder(height, width, framerate, samplerate));
        encoders[i]->init_encoder(IniterOs[i]->get_fmt_ctx());

        IniterOs[i]->ofmt_io_open();
        IniterOs[i]->print_ofmt_info();
    }

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
    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "swr_ctx init failed.\n");
        return -1;
    }

    fifo = av_audio_fifo_alloc(encoders[0]->get_audio_enc_ctx()->sample_fmt, encoders[0]->get_audio_enc_ctx()->channels, 1);
    if(!fifo){
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
        encoders[i]->init_encoder(IniterOs[i]->get_fmt_ctx());

        IniterOs[i]->ofmt_io_open();
        IniterOs[i]->print_ofmt_info();
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




//转码转文件MP4函数(单文件)
int Transcoder::encode_onefile_as_mp4(int& work){
    double count_video = 0.0;
    double count_audio = 0.0;
    bool all_finish;
    while(true){
        all_finish = ( work == 0) && decoders[0]->get_audio_queue().empty() && decoders[0]->get_video_queue().empty() &&  av_audio_fifo_size(fifo) == 0;
        if(all_finish) break;

        int time = 5;
        while(time != 0) {
            time--;
            if(!decoders[0]->get_audio_queue().empty()) {
                Utils::write_to_fifo(fifo, swr_ctx, av_frame_clone(decoders[0]->get_audio_queue().front()),
                                     encoders[0]->get_audio_enc_ctx()->channels,
                                     encoders[0]->get_audio_enc_ctx()->sample_fmt,
                                     encoders[0]->get_audio_enc_ctx()->sample_rate);
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[0]->pop_audio();
                }
            }
        }

        if(av_audio_fifo_size(fifo) > 0) {
            int ret = encoders[0]->encode(Utils::get_sample_fixed_frame(fifo,encoders[0]->get_audio_enc_ctx(), true), 1,
                                          count_audio);
            if (ret < 0) return -1;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            if(!decoders[0]->get_video_queue().empty()){
                int ret = encoders[0]->encode(Utils::change_frame_size(width, height, av_frame_clone(decoders[0]->get_video_queue().front())), 0, count_video);
                if(ret < 0) return -1;
                decoders[0]->pop_video();
            }
        }

        {
            std::unique_lock<std::mutex> lock(mtx);
            cond.notify_all();
        }

    }

    encoders[0]->encode(nullptr, 0, count_video);
    encoders[0]->encode(nullptr, 1, count_audio);

    return 0;
}



//转码推流rtmp函数(单文件)
int Transcoder::encode_onefile_as_rtmp(int &work) {
    int64_t last_time = -1;
    AVFrame *last_frame = FrameCreater::create_video_frame();
    bool all_finish;

    double count_ratio = (44100 * 1.0 / 1024 - framerate) / framerate;
    double count = 0.0;

    while(true){
        all_finish = (work == 0) && decoders[0]->get_audio_queue().empty()
                                && decoders[0]->get_video_queue().empty()
                                && av_audio_fifo_size(fifo) == 0;
        if(all_finish) break;


        int time = 5;
        while(time != 0) {
            time--;
            if(!decoders[0]->get_audio_queue().empty()) {
                Utils::write_to_fifo(fifo, swr_ctx, av_frame_clone(decoders[0]->get_audio_queue().front()),
                                     encoders[0]->get_audio_enc_ctx()->channels,
                                     encoders[0]->get_audio_enc_ctx()->sample_fmt,
                                     encoders[0]->get_audio_enc_ctx()->sample_rate);
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[0]->pop_audio();
                }
            }
        }



        if(count >= 1){
            count -= 1.0;
            if(av_audio_fifo_size(fifo) > 0) {
                int ret = encoders[0]->encode_audio(Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), true));
                if (ret < 0) return -1;
            }
        }
        if (!decoders[0]->get_video_queue().empty()) {
            encoders[0]->encode_video(Utils::change_frame_size(1920, 1080, av_frame_clone(decoders[0]->get_video_queue().front())), last_time);
            av_frame_free(&last_frame);
            last_frame = av_frame_clone(decoders[0]->get_video_queue().front());
            {
                std::lock_guard<std::mutex> lock(mtx);
                decoders[0]->pop_video();
            }
            {
                std::unique_lock<std::mutex> lock(mtx);
                cond.notify_all();
            }

        } else {
            encoders[0]->encode_video(Utils::change_frame_size(width, height, av_frame_clone(last_frame)), last_time);
        }

        if(av_audio_fifo_size(fifo) > 0) {
            int ret = encoders[0]->encode_audio(Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), true));
            if (ret < 0) return -1;
        }
        count += count_ratio;

    }
    av_usleep(7000000);
    return 0;
}


//多文件转码推流rtmp
int Transcoder::encode_files_as_rtmp() {
    //创建解码线程 并发执行解码
    std::vector<int> works(inputNums, 1);
    for(int i = 0; i < inputNums; i++){
        Thread thread(&Decoder::decode, decoders[i], std::ref(works[i]));
        thread.detach();
    }

    //构造视频帧 编码需要的内容：合并编码帧、前一帧
    int64_t last_time = -1;
    std::vector<AVFrame *> last_frames(inputNums);
    std::vector<AVFrame *> merge_frames(inputNums);
    for(int i = 0; i < last_frames.size(); i++){
        last_frames[i] = FrameCreater::create_video_frame();
        merge_frames[i] = FrameCreater::create_video_frame();
    }
    bool all_finish;

    //计算音频帧和视频帧之间比率 通过比率计算实现同步
    double count_ratio = (samplerate * 1.0 / encoders[0]->get_audio_enc_ctx()->frame_size - framerate) / framerate;
    double count = 0.0;

    bool all_audio_finish;
    while(true){

        //判断退出的条件
        all_audio_finish = true;
        all_finish = true;
        for(int i = 0; i < inputNums; i++){
            all_finish =  all_finish && (works[i] == 0) &&
                          (decoders[i]->get_video_queue().empty()) &&
                          (decoders[i]->get_audio_queue().empty());
        }
        all_finish = all_finish && av_audio_fifo_size(fifo) == 0;
        if(all_finish) break;


        //首先需要从音频解码队列中取数据并通过重采样存储到av_audio_fifo中 这里给到一个计数是为了保证fifo中至少有一定量的缓存
        int time = 5;
        while(time != 0) {
            time--;
            if(!decoders[0]->get_audio_queue().empty()) {
                Utils::write_to_fifo(fifo, swr_ctx, av_frame_clone(decoders[0]->get_audio_queue().front()),
                                     encoders[0]->get_audio_enc_ctx()->channels,
                                     encoders[0]->get_audio_enc_ctx()->sample_fmt,
                                     encoders[0]->get_audio_enc_ctx()->sample_rate);
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[0]->pop_audio();
                }
            }
        }

        //判断是否所有解码帧都已经使用
        all_audio_finish = works[0] == 0 && decoders[0]->get_audio_queue().empty();

        //count >= 1.0说明需要多编码一次音频 个人感觉应当换为while----todo
        //if(count >= 1){
        while(count >= 1){
            count -= 1.0;

            //从fifo中提取音频数据并编码音频帧
            if(av_audio_fifo_size(fifo) > 0){
                AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
                if(audio_frame){
                    int ret = encoders[0]->encode_audio(audio_frame);
                    if(ret < 0) return -1;
                }
            }

        }

        //依此处理多个解码队列中的视频帧 如果同一时间有解码队列为空 则使用上一帧
        for(int i = 0; i < inputNums; i++){
            if(!decoders[i]->get_video_queue().empty()){
                av_frame_free(&last_frames[i]);
                last_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[i]->pop_video();
                }

                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cond.notify_all();
                }
            } else{
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(last_frames[i]);
            }

            //因为暂时只用到了第一个视频的音频 所以这里还需要把其余音频解码队列中帧释放掉
            while(i != 0 && !decoders[i]->get_audio_queue().empty()){
                std::lock_guard<std::mutex> lock(mtx);
                decoders[i]->pop_audio();
            }

        }
        //编码合并后的视频帧
        encoders[0]->encode_video(Utils::merge_way(width, height, merge_frames), last_time);


        //从fifo中提取数据并编码为音频帧发送 同时计数器加比率
        if(av_audio_fifo_size(fifo) > 0){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
            if(audio_frame){
                int ret = encoders[0]->encode_audio(audio_frame);
                if(ret < 0) return -1;
            }
        }
        count += count_ratio;
    }

    encoders[0]->encode_audio(nullptr);

    //释放堆区创建的内存
    for(int i = 0; i < inputNums; i++){
        av_frame_free(&last_frames[i]);
        av_frame_free(&merge_frames[i]);
    }
    while(!last_frames.empty()){
        last_frames.pop_back();
        merge_frames.pop_back();
    }

    //这里sleep是为了让拉流端能顺利读完所有数据
    av_usleep(7000000);
    return 0;
}


//多文件转码推流mp4
int Transcoder::encode_files_as_mp4() {
    //新添加需求 假如有多个视频 选择最长的视频 并且视频帧结束后 不再编码过去的帧 完成
    //判断方法优化 根据inputNums动态分析视频个数 完成
    //更换逻辑 转完一类 再转一类（可能导致某一种类型队列塞满）

    //音频随意转换高转低低转高均可完成 低采样率转高采样率最后几帧音频会丢失 --->思路：修改帧采样大小x
    //目前最多支持四个视频 后续可以动态增添（有难度）

    //初始化works 启动多个线程执行解码
    std::vector<int> works(inputNums, 1);
    for(int i = 0; i < inputNums; i++){
        Thread thread(&Decoder::decode, decoders[i], std::ref(works[i]));
        thread.detach();
    }

    //初始化视频帧相关帧
    std::vector<AVFrame *> last_frames(inputNums);
    std::vector<AVFrame *> merge_frames(inputNums);
    for(int i = 0; i < last_frames.size(); i++){
        last_frames[i] = FrameCreater::create_video_frame();
        merge_frames[i] = FrameCreater::create_video_frame();
    }
    bool all_audio_finish;
    bool all_finish;

    while(true){
        //判断终止条件 判断是否所有读取完毕
        all_finish = true;
        for(int i = 0; i < inputNums; i++){
            all_finish =  all_finish &&
                          (works[i] == 0) &&
                          (decoders[i]->get_video_queue().empty()) &&
                          (decoders[i]->get_audio_queue().empty());
        }
        all_finish = all_finish && av_audio_fifo_size(fifo) == 0;
        if(all_finish) break;


        //一定量的计数保证av_audio_fifo中存储一定量的音频帧数据
        int time = 5;
        while(time != 0) {
            time--;
            if(!decoders[0]->get_audio_queue().empty()) {
                Utils::write_to_fifo(fifo, swr_ctx,
                                     av_frame_clone(decoders[0]->get_audio_queue().front()),
                                     encoders[0]->get_audio_enc_ctx()->channels,
                                     encoders[0]->get_audio_enc_ctx()->sample_fmt,
                                     encoders[0]->get_audio_enc_ctx()->sample_rate);
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[0]->pop_audio();
                }
            }
        }

        //依此提取不同视频帧 如果某个视频暂时没有帧 则用前一个帧替代
        bool video_queues_empty = true;
        for(int i = 0; i < inputNums; i++){
            if(!decoders[i]->get_video_queue().empty()){
                video_queues_empty = false;
                av_frame_free(&last_frames[i]);
                last_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[i]->pop_video();
                }

                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cond.notify_all();
                }
            } else{
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(last_frames[i]);
            }

            //因为暂时只用到了第一个视频的音频 这里还需要视频其余音频
            if(i != 0){
                while(!decoders[i]->get_audio_queue().empty()){
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[i]->pop_audio();
                }
            }
        }
        //判断是否需要继续编码视频 防止提前结束
        if(!video_queues_empty) encoders[0]->encode_video_mp4(Utils::merge_way(width, height, merge_frames));
        all_audio_finish = works[0] == 0 && decoders[0]->get_audio_queue().empty();


        //从fifo中提取数据并编码音频帧
        while(av_audio_fifo_size(fifo) > 0){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
            if(audio_frame){
                int ret = encoders[0]->encode_audio(audio_frame);
                if(ret < 0) return -1;
            } else break;
        }

    }

    //处理最后几帧
    encoders[0]->encode_video_mp4(nullptr);
    encoders[0]->encode_audio(nullptr);

    //释放之间申请的内存
    for(int i = 0; i < inputNums; i++){
        av_frame_free(&last_frames[i]);
        av_frame_free(&merge_frames[i]);
    }
    while(!last_frames.empty()){
        last_frames.pop_back();
        merge_frames.pop_back();
    }

    return 0;
}


int Transcoder::encode_files_rtmp_threads() {
    //初始化works 并创建多个线程执行decode
    unsigned int i;
    std::vector<int> works(inputNums, 1);
    for(i = 0; i < inputNums; i++){
        Thread thread(&Decoder::decode, decoders[i], std::ref(works[i]));
        thread.detach();
    }

    double count_video = 0.0;
    double count_audio = 0.0;

    //新开线程编码视频
    Thread thread_video(&Transcoder::encode_thread_video_rtmp, this, std::ref(works), std::ref(count_video), std::ref(count_audio));
    //thread_video.detach();

    //新开线程编码音频
    Thread thread_audio(&Transcoder::encode_thread_audio_rtmp, this, std::ref(works), std::ref(count_video), std::ref(count_audio));
    //thread_audio.detach();

    //计算比率/计算个数/计算一帧播放的时间
    // 一帧音频播放时间 1024 / samplerate * 1000 ms    一帧视频播放时间 1 * 1000 / framerate ms
    // 一秒钟音频帧个数 samplerate / framesize    一秒钟视频帧个数 framerate

    //开启线程编码视频 根据传入的比率或者参数进行调整

    //开启线程编码视频 根据传入的比率或者参数进行调整

    return 0;

}

int Transcoder::encode_thread_video_rtmp(std::vector<int> &works, double &count_video, double &count_audio) {
    //初始化视频帧编码需要的内容 前一帧+合并帧
    unsigned int i;
    bool video_finish;
    int64_t last_time = -1;
    std::vector<AVFrame *> last_frames(inputNums);
    std::vector<AVFrame *> merge_frames(inputNums);
    for(int i = 0; i < last_frames.size(); i++){
        last_frames[i] = FrameCreater::create_video_frame();
        merge_frames[i] = FrameCreater::create_video_frame();
    }

    //计算一帧视频的时间
    double per_frame_video = (1.0 * 1000) / framerate;

    while(true){

        //判断视频解码是否全部结束
        video_finish = true;
        for(i = 0; i < inputNums; i++){
            video_finish = video_finish && (decoders[i]->get_video_queue().empty()) && (works[i] == 0);
        }
        if(video_finish) break;


        //依此提取多个解码队列中的视频帧 并合并一起编码
        for(i = 0; i < inputNums; i++){
            if(!decoders[i]->get_video_queue().empty()){
                av_frame_free(&last_frames[i]);
                last_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[i]->pop_video();
                }

                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cond.notify_all();
                }
            } else{
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(last_frames[i]);
            }
        }

        //判断音频编码未结束的时候 否则可能会一直等待
        if(!((works[0] == 0) && decoders[0]->get_audio_queue().empty() && !av_audio_fifo_size(fifo) > 0))
        {
            //条件变量设置 当视频帧时间戳 <= 音频时间戳时释放
            std::unique_lock<std::mutex> lock(mtx2);
            while(count_video > count_audio){
                cond2.wait(lock);
            }
        }

        //编码视频帧
        {
            std::lock_guard<std::mutex> lock(mtx2);
            encoders[0]->encode_video(Utils::merge_way(width, height, merge_frames), last_time);
            count_video += per_frame_video;
        }
    }
    encoders[0]->encode_video_mp4(nullptr);

    //释放申请的内存
    for(int i = 0; i < inputNums; i++){
        av_frame_free(&last_frames[i]);
        av_frame_free(&merge_frames[i]);
    }
    while(!last_frames.empty()){
        last_frames.pop_back();
        merge_frames.pop_back();
    }

    av_usleep(7000000);

    return 0;

}


int Transcoder::encode_thread_audio_rtmp(std::vector<int> &works, double &count_video, double &count_audio) {
    int ret;
    unsigned int i;
    bool audio_finish;
    bool all_audio_finish;

    //计算一帧音频播放的时间
    double per_frame_audio = encoders[0]->get_audio_enc_ctx()->frame_size * 1000 * 1.0 / samplerate;

    while(true){

        //判断是否所有音频帧结束
        audio_finish = true;
        for(i = 0; i < inputNums; i++){
            audio_finish = audio_finish && (decoders[i]->get_audio_queue().empty()) && (works[i] == 0);
        }
        if(audio_finish) break;

        //使用计数是为了保证fifo中存足够的音频数据
        int time = 5;
        while(time--){
            for(i = 0; i < inputNums; i++){
                if(i == 0){
                    if(!decoders[i]->get_audio_queue().empty()) {
                        Utils::write_to_fifo(fifo, swr_ctx,
                                             av_frame_clone(decoders[i]->get_audio_queue().front()),
                                             encoders[i]->get_audio_enc_ctx()->channels,
                                             encoders[i]->get_audio_enc_ctx()->sample_fmt,
                                             encoders[i]->get_audio_enc_ctx()->sample_rate);
                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            decoders[i]->pop_audio();
                        }
                    }
                } else{
                    //因为暂时只用到了第一个视频的音频 所以其余音频帧需要被释放
                    while(!decoders[i]->get_audio_queue().empty()){
                        std::lock_guard<std::mutex> lock(mtx);
                        decoders[i]->pop_audio();
                    }
                }
            }
        }

        all_audio_finish = works[0] == 0 && decoders[0]->get_audio_queue().empty();
        //当fifo中有足够数据时 进行编码音频
        while(av_audio_fifo_size(fifo) >= encoders[0]->get_audio_enc_ctx()->frame_size || (av_audio_fifo_size(fifo) > 0 && all_audio_finish)){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
            if(audio_frame){
                {
                    std::lock_guard<std::mutex> lock(mtx2);
                    ret = encoders[0]->encode_audio(audio_frame);
                    count_audio += per_frame_audio;
                }
                {
                    std::unique_lock<std::mutex> lock(mtx2);
                    cond2.notify_all();
                }

                if(ret < 0) return -1;
            } else break;
        }

    }
    encoders[0]->encode_audio(nullptr);
    return 0;

}


int Transcoder::encode_files_mp4_threads() {
    unsigned int i;
    std::vector<int> works(inputNums, 1);
    for(i = 0; i < inputNums; i++){
        Thread thread(&Decoder::decode, decoders[i], std::ref(works[i]));
        thread.detach();
    }

    //新开一个线程编码视频
    Thread thread_video(&Transcoder::encode_thread_video_mp4, this, std::ref(works));

    //新开一个线程编码音频
    Thread thread_audio(&Transcoder::encode_thread_audio_mp4, this, std::ref(works));

    return 0;

}



int Transcoder::encode_thread_video_mp4(std::vector<int>& works) {
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
            video_finish = video_finish && (decoders[i]->get_video_queue().empty()) && (works[i] == 0);
        }
        if(video_finish) break;

        for(i = 0; i < inputNums; i++){
            if(!decoders[i]->get_video_queue().empty()){
                av_frame_free(&last_frames[i]);
                last_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[i]->pop_video();
                }

                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cond.notify_all();
                }
            } else{
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(last_frames[i]);
            }
        }
        {
            std::lock_guard<std::mutex> lcok(mtx2);
            encoders[0]->encode_video_mp4(Utils::merge_way(width, height, merge_frames));
        }
    }
    encoders[0]->encode_video_mp4(nullptr);

    for(int i = 0; i < inputNums; i++){
        av_frame_free(&last_frames[i]);
        av_frame_free(&merge_frames[i]);
    }
    while(!last_frames.empty()){
        last_frames.pop_back();
        merge_frames.pop_back();
    }

    return 0;
}

int Transcoder::encode_thread_audio_mp4(std::vector<int>& works) {
    int ret;
    unsigned int i;
    bool audio_finish;
    bool all_audio_finish;
    while(true){
        audio_finish = true;
        for(i = 0; i < inputNums; i++){
            audio_finish = audio_finish && (decoders[i]->get_audio_queue().empty()) && (works[i] == 0);
        }
        if(audio_finish) break;

        int time = 5;
        while(time--){
            for(i = 0; i < inputNums; i++){
                if(i == 0){
                    if(!decoders[i]->get_audio_queue().empty()) {
                        Utils::write_to_fifo(fifo, swr_ctx,
                                             av_frame_clone(decoders[i]->get_audio_queue().front()),
                                             encoders[i]->get_audio_enc_ctx()->channels,
                                             encoders[i]->get_audio_enc_ctx()->sample_fmt,
                                             encoders[i]->get_audio_enc_ctx()->sample_rate);
                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            decoders[i]->pop_audio();
                        }
                    }
                } else{
                    while(!decoders[i]->get_audio_queue().empty()){
                        std::lock_guard<std::mutex> lock(mtx);
                        decoders[i]->pop_audio();
                    }
                }
            }
        }

        all_audio_finish = works[0] == 0 && decoders[0]->get_audio_queue().empty();
        while(av_audio_fifo_size(fifo) > 0){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
            if(audio_frame){
                {
                    std::lock_guard<std::mutex> lock(mtx2);
                    ret = encoders[0]->encode_audio(audio_frame);
                }
                if(ret < 0) return -1;
            } else break;
        }
    }
    encoders[0]->encode_audio(nullptr);
    return 0;
}


int Transcoder::reencode() {
    int ret;

    Writer::write_header(IniterOs[0]->get_fmt_ctx());


    if(output_filenames[0].substr(0, 4) == "rtmp"){
       // ret = encode_files_as_rtmp();
      // ret = encode_files_rtmp_threads();
       //ret = encode_files_rtmp_test();
       ret = encode_thread_rtmp();
    }

    if(output_filenames[0].substr(output_filenames[0].size() - 4, 4) == ".mp4"){
        //ret = encode_files_as_mp4();
        //ret = encode_files_mp4_threads();
        ret = encode_thread_mp4();
    }


    if(output_filenames[0].substr(0, 4) == "rtsp"){
        ret = encode_thread_rtsp();
    }



    Writer::write_tail(IniterOs[0]->get_fmt_ctx());

    return 0;
}


int Transcoder::reencode_local_device() {
    int ret;

    Writer::write_header(IniterOs[0]->get_fmt_ctx());
    //Write--->packets

    if(output_filenames[0].substr(0, 4) == "rtmp"){
        // ret = encode_files_as_rtmp();
        //ret = encode_files_rtmp_threads();
       // ret = encode_device_mp4();
       ret = encode_device_rtmp();
    }
    if(output_filenames[0].substr(output_filenames[0].size() - 4, 4) == ".mp4"){
        //ret = encode_files_as_mp4();
        //ret = encode_files_mp4_threads();
        ret = encode_device_mp4();
    }




    Writer::write_tail(IniterOs[0]->get_fmt_ctx());
    return 0;


}



int Transcoder::encode_device_mp4() {
    int video_over = 0;
    int audio_over = 0;

    //int all_over = 0;

    Thread thread_decode_video(&Decoder::test_decode_only_video, decoders[0], std::ref(video_over));

    //Thread thread_decode_audio(&Decoder::test_decode_only_audio, decoders[1], std::ref(audio_over));


    //设置视频的时长
    int time_s = 15;

    Thread thread_encode_video(&Transcoder::encode_thread_video_deivce_mp4_separate, this, std::ref(time_s), std::ref(video_over));
    //thread_encode_video.detach();

    //Thread thread_encode_audio(&Transcoder::encode_thread_audio_device_mp4_separate, this, std::ref(time_s), std::ref(audio_over));
    //thread_encode_audio.detach();

    Thread thread_encode_audio(&Transcoder::encode_thread_audio_mp4_silent_frames_separate, this, std::ref(time_s));

   //while(video_over == 0 || audio_over == 0) continue;
   return 0;
}


int Transcoder::encode_device_rtmp() {
//初始化works 并创建多个线程执行decode
    unsigned int i;


    int video_over = 0;
    int audio_over = 0;


    Thread thread_decode_video(&Decoder::test_decode_only_video, decoders[0], std::ref(video_over));
    //thread_decode_video.detach();

//    Thread thread_decode_audio(&Decoder::test_decode_only_audio, decoders[1], std::ref(audio_over));
//    thread_decode_audio.detach();


    int time = 600;



    Thread thread_video(&Transcoder::encode_thread_video_rtmp_device_separate, this, std::ref(time), std::ref(video_over));

    Thread thread_audio(&Transcoder::encode_thread_audio_rtmp_silent_frames_separate, this, std::ref(time));
    //Thread thread_audio(&Transcoder::encode_thread_audio_rtmp_device_separate, this, std::ref(time), std::ref(audio_over));


    return 0;
}



int Transcoder::encode_files_rtmp_test() {
    int ret;
    int work = 1;
    int64_t last_time = -1;
    AVFrame *last_frame = FrameCreater::create_video_frame();
    bool all_finish;

    Thread thread_decode(&Decoder::decode, decoders[0], std::ref(work));




//    double count_ratio = (44100 * 1.0 / 1024 - framerate) / framerate;
//    double count = 0.0;

    double count_audio = 0.0;
    double count_video = 0.0;

    while(true){
        all_finish = (work == 0) && decoders[0]->get_audio_queue().empty()
                     && decoders[0]->get_video_queue().empty()
                     && av_audio_fifo_size(fifo) == 0;
        if(all_finish) break;


        int time = 5;
        while(time != 0) {
            time--;
            if(!decoders[0]->get_audio_queue().empty()) {
                Utils::write_to_fifo(fifo, swr_ctx, av_frame_clone(decoders[0]->get_audio_queue().front()),
                                     encoders[0]->get_audio_enc_ctx()->channels,
                                     encoders[0]->get_audio_enc_ctx()->sample_fmt,
                                     encoders[0]->get_audio_enc_ctx()->sample_rate);
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[0]->pop_audio();
                }
            }
        }

        if(count_audio <= count_video){
            if(av_audio_fifo_size(fifo) > 0) {
                ret = encoders[0]->encode(Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), true), 1, count_audio);
                //int ret = encoders[0]->encode_audio(Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), true));
                if (ret < 0) return -1;
            }
        } else{
            if(!decoders[0]->get_video_queue().empty()){
                ret = encoders[0]->encode(Utils::change_frame_size(width, height, av_frame_clone(decoders[0]->get_video_queue().front())), 0, count_video);
//                av_frame_free(&last_frame);
//                last_frame = av_frame_clone(decoders[0]->get_video_queue().front());
                //ret = encoders[0]->encode_video(Utils::change_frame_size(width, height, av_frame_clone(decoders[0]->get_video_queue().front())), last_time);
                //count_video += 33.3333;
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[0]->pop_video();
                }
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cond.notify_all();
                }

            }
        }



//        if(count >= 1){
//            count -= 1.0;
//            if(av_audio_fifo_size(fifo) > 0) {
//                int ret = encoders[0]->encode_audio(Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), true));
//                if (ret < 0) return -1;
//            }
//        }
//        if (!decoders[0]->get_video_queue().empty()) {
//            encoders[0]->encode_video(Utils::change_frame_size(1920, 1080, av_frame_clone(decoders[0]->get_video_queue().front())), last_time);
//            av_frame_free(&last_frame);
//            last_frame = av_frame_clone(decoders[0]->get_video_queue().front());
//            {
//                std::lock_guard<std::mutex> lock(mtx);
//                decoders[0]->pop_video();
//            }
//            {
//                std::unique_lock<std::mutex> lock(mtx);
//                cond.notify_all();
//            }
//
//        } else {
//            encoders[0]->encode_video(Utils::change_frame_size(width, height, av_frame_clone(last_frame)), last_time);
//        }
//
//        if(av_audio_fifo_size(fifo) > 0) {
//            int ret = encoders[0]->encode_audio(Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), true));
//            if (ret < 0) return -1;
//        }
//        count += count_ratio;

    }
    av_usleep(7000000);
    return 0;
}


int Transcoder::encode_thread_rtmp() {
    unsigned int i;
    int ret;

    std::vector<int> works(inputNums, 1);
    for(i = 0; i < inputNums; i++){
        Thread thread_decode(&Decoder::decode, decoders[i], std::ref(works[i]));
        thread_decode.detach();
    }

    Thread thread_encode_video(&Transcoder::encode_thread_video_rtmp_separate, this, std::ref(works));

    Thread thread_encode_audio(&Transcoder::encode_thread_audio_rtmp_separate, this, std::ref(works));

    return 0;
}




int Transcoder::encode_thread_video_rtmp_new(std::vector<int> &works, double &count_video, double &count_audio) {
    //初始化视频帧编码需要的内容 前一帧+合并帧
    unsigned int i;
    bool video_finish;
    int64_t last_time = -1;

    int64_t time_one_second = Timer::getCurrentTime();
    int count_video_frames = 0;


    std::vector<AVFrame *> last_frames(inputNums);
    std::vector<AVFrame *> merge_frames(inputNums);
    for(int i = 0; i < last_frames.size(); i++){
        last_frames[i] = FrameCreater::create_video_frame();
        merge_frames[i] = FrameCreater::create_video_frame();
    }

    //计算一帧视频的时间
    double per_frame_video = (1.0 * 1000) / framerate;

    while(true){

        if((Timer::getCurrentTime() - time_one_second) / 1000 / 1000 >= 1){
            av_log(nullptr, AV_LOG_INFO, "one second counts video: %d.\n", count_video_frames);
            count_video_frames = 0;
            time_one_second = Timer::getCurrentTime();
        }

        //判断视频解码是否全部结束
        video_finish = true;
        for(i = 0; i < inputNums; i++){
            video_finish = video_finish && (decoders[i]->get_video_queue().empty()) && (works[i] == 0);
        }
        if(video_finish) break;


        //依此提取多个解码队列中的视频帧 并合并一起编码
        for(i = 0; i < inputNums; i++){
            if(!decoders[i]->get_video_queue().empty()){
                av_frame_free(&last_frames[i]);
                last_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[i]->pop_video();
                }

                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cond.notify_all();
                }
            } else{
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(last_frames[i]);
            }
        }

        //判断音频编码未结束的时候 否则可能会一直等待
        if(!((works[0] == 0) && decoders[0]->get_audio_queue().empty() && !av_audio_fifo_size(fifo) > 0)){
            //条件变量设置 当视频帧时间戳 <= 音频时间戳时释放
            std::unique_lock<std::mutex> lock(mtx2);
            while(count_video >= count_audio){
                cond2.wait(lock);
            }
        }

        //编码视频帧
        {
            std::lock_guard<std::mutex> lock(mtx2);
            encoders[0]->encode_video_rtmp(Utils::merge_way(width, height, merge_frames), count_video, last_time);
            //encoders[0]->encode_video(Utils::merge_way(width, height, merge_frames), last_time);
            //count_video += per_frame_video;
        }
        {
            std::unique_lock<std::mutex> locK(mtx2);
            cond2.notify_all();
        }


        count_video_frames++;
    }
    //encoders[0]->encode_video_mp4(nullptr);

    //释放申请的内存
    for(int i = 0; i < inputNums; i++){
        av_frame_free(&last_frames[i]);
        av_frame_free(&merge_frames[i]);
    }
    while(!last_frames.empty()){
        last_frames.pop_back();
        merge_frames.pop_back();
    }

    av_usleep(7000000);

    return 0;
}

int Transcoder::encode_thread_audio_rtmp_new(std::vector<int> &works, double &count_video, double &count_audio) {
    int ret;
    unsigned int i;
    AVFrame* silence_frame = FrameCreater::create_audio_frame();
    int64_t time_one_second = Timer::getCurrentTime();
    int count_audio_frames = 0;

    int64_t last_time = -1;
    bool audio_finish;
    bool all_audio_finish;

    //计算一帧音频播放的时间
    double per_frame_audio = encoders[0]->get_audio_enc_ctx()->frame_size * 1000 * 1.0 / samplerate;

    while(true){
        if((Timer::getCurrentTime() - time_one_second) / 1000 / 1000 >= 1){
            av_log(nullptr, AV_LOG_INFO, "one second counts audio: %d.\n", count_audio_frames);
            count_audio_frames = 0;
            time_one_second = Timer::getCurrentTime();
        }

        //判断是否所有音频帧结束
        audio_finish = true;
        for(i = 0; i < inputNums; i++){
            audio_finish = audio_finish && (decoders[i]->get_audio_queue().empty()) && (works[i] == 0);
        }
        if(audio_finish) break;

        //使用计数是为了保证fifo中存足够的音频数据
        int time = 5;
        while(time--){
            for(i = 0; i < inputNums; i++){
                if(i == 0){
                    if(!decoders[i]->get_audio_queue().empty()) {
                        Utils::write_to_fifo(fifo, swr_ctx,
                                             av_frame_clone(decoders[i]->get_audio_queue().front()),
                                             encoders[i]->get_audio_enc_ctx()->channels,
                                             encoders[i]->get_audio_enc_ctx()->sample_fmt,
                                             encoders[i]->get_audio_enc_ctx()->sample_rate);

                        {
                            std::unique_lock<std::mutex> lock(mtx);
                            cond.notify_all();
                        }

                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            decoders[i]->pop_audio();
                        }
                    }
                } else{
                    //因为暂时只用到了第一个视频的音频 所以其余音频帧需要被释放
                    while(!decoders[i]->get_audio_queue().empty()){
                        std::lock_guard<std::mutex> lock(mtx);
                        decoders[i]->pop_audio();
                    }
                }
            }
        }

        all_audio_finish = works[0] == 0 && decoders[0]->get_audio_queue().empty();
        if(!((works[0] == 0 && decoders[0]->get_video_queue().empty())))
        {
            std::unique_lock<std::mutex> lock(mtx2);
            while(count_audio > count_video){
                cond2.wait(lock);
            }
        }

        if(av_audio_fifo_size(fifo) >= encoders[0]->get_audio_enc_ctx()->frame_size || (av_audio_fifo_size(fifo) > 0 && all_audio_finish)){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
            if(audio_frame){
                {
                    std::lock_guard<std::mutex> lock(mtx2);
                    //ret = encoders[0]->encode_audio(audio_frame);
                    ret = encoders[0]->encode_audio_rtmp(audio_frame, count_audio, last_time);
                    //count_audio += per_frame_audio;
                }
                {
                    std::unique_lock<std::mutex> lock(mtx2);
                    cond2.notify_all();
                }

                if(ret < 0) return -1;
            } else break;
        } else{
            {
                std::lock_guard<std::mutex> lock(mtx2);
                ret = encoders[0]->encode_audio_rtmp(av_frame_clone(silence_frame), count_audio ,last_time);
            }
            {
                std::unique_lock<std::mutex> lock(mtx2);
                cond2.notify_all();
            }
        }
        count_audio_frames++;

//        //当fifo中有足够数据时 进行编码音频
//        while(av_audio_fifo_size(fifo) >= encoders[0]->get_audio_enc_ctx()->frame_size || (av_audio_fifo_size(fifo) > 0 && all_audio_finish)){
//            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
//            if(audio_frame){
//                {
//                    std::lock_guard<std::mutex> lock(mtx2);
//                    ret = encoders[0]->encode_audio(audio_frame);
//                    count_audio += per_frame_audio;
//                }
//                {
//                    std::unique_lock<std::mutex> lock(mtx2);
//                    cond2.notify_all();
//                }
//
//                if(ret < 0) return -1;
//            } else break;
//        }

    }
    encoders[0]->encode_audio(nullptr);
    return 0;
}




int Transcoder::encode_thread_video_rtmp_separate(std::vector<int>& works) {
    //初始化视频帧编码需要的内容 前一帧+合并帧
    unsigned int i;
    bool video_finish;
    //int64_t last_time = -1;

    int64_t time_one_second = Timer::getCurrentTime();
    int count_video_frames = 0;


    std::vector<AVFrame *> last_frames(inputNums);
    std::vector<AVFrame *> merge_frames(inputNums);
    for(int i = 0; i < last_frames.size(); i++){
        last_frames[i] = FrameCreater::create_video_frame();
        merge_frames[i] = FrameCreater::create_video_frame();
    }

    //计算一帧视频的时间
    double per_frame_video = (1.0 * 1000) / framerate;

    while(true){
        if((Timer::getCurrentTime() - time_one_second) / 1000 / 1000 >= 1){
            av_log(nullptr, AV_LOG_INFO, "one second counts video: %d.\n", count_video_frames);
            count_video_frames = 0;
            time_one_second = Timer::getCurrentTime();
        }

        if(count_video_frames >= (encoders[0]->get_video_enc_ctx()->framerate.num) / (encoders[0]->get_video_enc_ctx()->framerate.den) + 1){
            av_usleep(1000);
            continue;
        }

        //判断视频解码是否全部结束
        video_finish = true;
        for(i = 0; i < inputNums; i++){
            video_finish = video_finish && (decoders[i]->get_video_queue().empty()) && (works[i] == 0);
        }
        if(video_finish) break;


        //依此提取多个解码队列中的视频帧 并合并一起编码
        for(i = 0; i < inputNums; i++){
            if(!decoders[i]->get_video_queue().empty()){
                av_frame_free(&last_frames[i]);
                last_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[i]->pop_video();
                }

                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cond.notify_all();
                }
            } else{
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(last_frames[i]);
            }
        }

        //编码视频帧
        {
            std::lock_guard<std::mutex> lock(mtx2);
            encoders[0]->encode_video_rtmp(Utils::merge_way(width, height, merge_frames));
        }
        count_video_frames++;
    }
    encoders[0]->encode_video_rtmp(nullptr);
    //encoders[0]->encode_video_mp4(nullptr);

    //释放申请的内存
    for(int i = 0; i < inputNums; i++){
        av_frame_free(&last_frames[i]);
        av_frame_free(&merge_frames[i]);
    }
    while(!last_frames.empty()){
        last_frames.pop_back();
        merge_frames.pop_back();
    }

    av_usleep(7000000);

    return 0;


}


int Transcoder::encode_thread_audio_rtmp_separate(std::vector<int>& works) {
    int ret;
    unsigned int i;
    AVFrame* silence_frame = FrameCreater::create_audio_frame();
    int64_t time_one_second = Timer::getCurrentTime();
    int count_audio_frames = 0;

    bool audio_finish;
    bool all_audio_finish;

    //计算一帧音频播放的时间
    double per_frame_audio = encoders[0]->get_audio_enc_ctx()->frame_size * 1000 * 1.0 / samplerate;

    while(true){
        if((Timer::getCurrentTime() - time_one_second) / 1000 / 1000 >= 1){
            av_log(nullptr, AV_LOG_INFO, "one second counts audio: %d.\n", count_audio_frames);
            count_audio_frames = 0;
            time_one_second = Timer::getCurrentTime();
        }

        if(count_audio_frames >= encoders[0]->get_audio_enc_ctx()->sample_rate * 1.0 / encoders[0]->get_audio_enc_ctx()->frame_size){
            av_usleep(1000);
            continue;
        }

        //判断是否所有音频帧结束
        audio_finish = true;
        for(i = 0; i < inputNums; i++){
            audio_finish = audio_finish && (decoders[i]->get_audio_queue().empty()) && (works[i] == 0);
        }
        if(audio_finish) break;

        //使用计数是为了保证fifo中存足够的音频数据
        int time = 5;
        while(time--){
            for(i = 0; i < inputNums; i++){
                if(i == 0){
                    if(!decoders[i]->get_audio_queue().empty()) {
                        Utils::write_to_fifo(fifo, swr_ctx,
                                             av_frame_clone(decoders[i]->get_audio_queue().front()),
                                             encoders[i]->get_audio_enc_ctx()->channels,
                                             encoders[i]->get_audio_enc_ctx()->sample_fmt,
                                             encoders[i]->get_audio_enc_ctx()->sample_rate);

//                        {
//                            std::unique_lock<std::mutex> lock(mtx);
//                            cond.notify_all();
//                        }

                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            decoders[i]->pop_audio();
                        }
                    }
                } else{
                    //因为暂时只用到了第一个视频的音频 所以其余音频帧需要被释放
                    while(!decoders[i]->get_audio_queue().empty()){
                        std::lock_guard<std::mutex> lock(mtx);
                        decoders[i]->pop_audio();
                    }
                }
            }
        }

        all_audio_finish = works[0] == 0 && decoders[0]->get_audio_queue().empty();
        if(av_audio_fifo_size(fifo) >= encoders[0]->get_audio_enc_ctx()->frame_size || (av_audio_fifo_size(fifo) > 0 && all_audio_finish)){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
            if(audio_frame){
                {
                    std::lock_guard<std::mutex> lock(mtx2);
                    ret = encoders[0]->encode_audio_rtmp(audio_frame);
                    count_audio_frames++;
                }
                if(ret < 0) return -1;
            } else break;
        }

    }
    encoders[0]->encode_audio_rtmp(nullptr);
    //encoders[0]->encode_audio(nullptr);
    return 0;
}


int Transcoder::encode_thread_video_mp4_separate(std::vector<int> &works) {
    //初始化视频帧编码需要的内容 前一帧+合并帧
    unsigned int i;
    bool video_finish;

    std::vector<AVFrame *> last_frames(inputNums);
    std::vector<AVFrame *> merge_frames(inputNums);
    for(int i = 0; i < last_frames.size(); i++){
        last_frames[i] = FrameCreater::create_video_frame();
        merge_frames[i] = FrameCreater::create_video_frame();
    }

    while(true){

        //判断视频解码是否全部结束
        video_finish = true;
        for(i = 0; i < inputNums; i++){
            video_finish = video_finish && (decoders[i]->get_video_queue().empty()) && (works[i] == 0);
        }
        if(video_finish) break;


        //依此提取多个解码队列中的视频帧 并合并一起编码
        for(i = 0; i < inputNums; i++){
            if(!decoders[i]->get_video_queue().empty()){
                av_frame_free(&last_frames[i]);
                last_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[i]->pop_video();
                }

                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cond.notify_all();
                }
            } else{
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(last_frames[i]);
            }
        }
        //编码视频帧
        {
            std::lock_guard<std::mutex> lock(mtx2);
            encoders[0]->encode_video_mp4(Utils::merge_way(width, height, merge_frames));
        }
    }
    encoders[0]->encode_video_mp4(nullptr);

    //释放申请的内存
    for(int i = 0; i < inputNums; i++){
        av_frame_free(&last_frames[i]);
        av_frame_free(&merge_frames[i]);
    }
    while(!last_frames.empty()){
        last_frames.pop_back();
        merge_frames.pop_back();
    }

    return 0;
}



int Transcoder::encode_thread_audio_mp4_separate(std::vector<int> &works) {
    int ret;
    unsigned int i;

    bool audio_finish;
    bool all_audio_finish;

    while(true){
        //判断是否所有音频帧结束
        audio_finish = true;
        for(i = 0; i < inputNums; i++){
            audio_finish = audio_finish && (decoders[i]->get_audio_queue().empty()) && (works[i] == 0);
        }
        if(audio_finish) break;

        //使用计数是为了保证fifo中存足够的音频数据
        int time = 5;
        while(time--){
            for(i = 0; i < inputNums; i++){
                if(i == 0){
                    if(!decoders[i]->get_audio_queue().empty()) {
                        Utils::write_to_fifo(fifo, swr_ctx,
                                             av_frame_clone(decoders[i]->get_audio_queue().front()),
                                             encoders[i]->get_audio_enc_ctx()->channels,
                                             encoders[i]->get_audio_enc_ctx()->sample_fmt,
                                             encoders[i]->get_audio_enc_ctx()->sample_rate);

                        {
                            std::unique_lock<std::mutex> lock(mtx);
                            cond.notify_all();
                        }

                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            decoders[i]->pop_audio();
                        }
                    }
                } else{
                    //因为暂时只用到了第一个视频的音频 所以其余音频帧需要被释放
                    while(!decoders[i]->get_audio_queue().empty()){
                        std::lock_guard<std::mutex> lock(mtx);
                        decoders[i]->pop_audio();
                    }
                }
            }
        }

        all_audio_finish = works[0] == 0 && decoders[0]->get_audio_queue().empty();

        if(av_audio_fifo_size(fifo) >= encoders[0]->get_audio_enc_ctx()->frame_size || (av_audio_fifo_size(fifo) > 0 && all_audio_finish)){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
            if(audio_frame){
                {
                    std::lock_guard<std::mutex> lock(mtx2);
                    //ret = encoders[0]->encode_audio_rtmp(audio_frame);
                    ret = encoders[0]->encode_audio_mp4(audio_frame);
                }
                if(ret < 0) return -1;
            } else break;
        }

    }
    encoders[0]->encode_audio_mp4(nullptr);
    return 0;
}








int Transcoder::encode_thread_mp4() {
    unsigned int i;
    int ret;

    std::vector<int> works(inputNums, 1);
    for(i = 0; i < inputNums; i++){
        Thread thread_decode(&Decoder::decode, decoders[i], std::ref(works[i]));
        thread_decode.detach();
    }


    Thread thread_video(&Transcoder::encode_thread_video_mp4, this, std::ref(works));

    Thread thread_audio(&Transcoder::encode_thread_audio_mp4, this, std::ref(works));


    return 0;

}



int Transcoder::encode_thread_video_rtmp_device_separate(int &time, int& video_over) {
    int ret;
    //AVFrame *last_frame = FrameCreater::create_video_frame();

    int video_frames_counts = framerate * time;
    int per_second_counts = 0;
    int64_t time_per_second = Timer::getCurrentTime();

    while(video_frames_counts >= 0){
        if(Timer::getCurrentTime() - time_per_second  >= 1000000){
            av_log(nullptr, AV_LOG_INFO, "per second video frame nums: %d.\n", per_second_counts);
            per_second_counts = 0;
            time_per_second = Timer::getCurrentTime();
        }

        if(per_second_counts >= framerate + 1){
            av_usleep(1000);
            continue;
        }

        if(!decoders[0]->get_video_queue().empty()){
            //encoders[0]->encode_video_rtmp(Utils::change_frame_size(width, height, av_frame_clone(decoders[0]->get_video_queue().front())));
            {
                std::lock_guard<std::mutex> lock(mtx2);
                encoders[0]->encode_video_rtmp(Utils::change_frame_size(width, height, av_frame_clone(decoders[0]->get_video_queue().front())));
            }
            decoders[0]->pop_video();
            per_second_counts++;
            video_frames_counts--;
        }


        {
            std::unique_lock<std::mutex> lock(mtx);
            cond.notify_all();
        }



    }

    encoders[0]->encode_video_rtmp(nullptr);
    //av_frame_free(&last_frame);
    video_over = 1;
    av_usleep(7000000);
    return 0;
}


int Transcoder::encode_thread_audio_rtmp_device_separate(int &time, int& audio_over) {
    int ret;
    unsigned int i;


    int audio_frames_counts = encoders[0]->get_audio_enc_ctx()->sample_rate / encoders[0]->get_audio_enc_ctx()->frame_size * time;
    int per_second_counts = 0;
    int64_t time_per_second = Timer::getCurrentTime();

    while(audio_frames_counts >= 0){
        if((Timer::getCurrentTime() - time_per_second) >= 1000000){
            av_log(nullptr, AV_LOG_INFO, "one second counts audio: %d.\n", per_second_counts);
            per_second_counts = 0;
            time_per_second = Timer::getCurrentTime();
        }

        if(per_second_counts >= encoders[0]->get_audio_enc_ctx()->sample_rate * 1.0 / encoders[0]->get_audio_enc_ctx()->frame_size){
            av_usleep(1000);
            continue;
        }


        //使用计数是为了保证fifo中存足够的音频数据
        int time = 1;
        while(time--){
            if(!decoders[1]->get_audio_queue().empty()){
                Utils::write_to_fifo(fifo, swr_ctx,
                                     av_frame_clone(decoders[1]->get_audio_queue().front()),
                                     encoders[0]->get_audio_enc_ctx()->channels,
                                     encoders[0]->get_audio_enc_ctx()->sample_fmt,
                                     encoders[0]->get_audio_enc_ctx()->sample_rate);


//                {
//                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[1]->pop_audio();
//                }
            }
        }

        if(av_audio_fifo_size(fifo) >= encoders[0]->get_audio_enc_ctx()->frame_size){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), false);
            if(audio_frame){
                {
                    std::lock_guard<std::mutex> lock(mtx2);
                    ret = encoders[0]->encode_audio_rtmp(audio_frame);
                }
                audio_frames_counts--;
                per_second_counts++;
                if(ret < 0) return -1;
            } else break;
        }

    }
    audio_over = 1;
    encoders[0]->encode_audio_rtmp(nullptr);
    return 0;

}


int Transcoder::encode_thread_video_deivce_mp4_separate(int& time, int& video_over) {
    int ret;
    int video_frames_counts = framerate * time;
    //AVFrame* last_frame = FrameCreater::create_video_frame();
    int64_t per_second_time = Timer::getCurrentTime();
    int per_second_counts = 0;

    while(video_frames_counts >= 0){
        if(Timer::getCurrentTime() - per_second_time >= 1000000){
            per_second_counts = 0;
            per_second_time = Timer::getCurrentTime();
        }

        if(per_second_counts >= framerate + 1){
            av_usleep(1000);
            continue;
        }

        if(!decoders[0]->get_video_queue().empty()){
            {
                std::lock_guard<std::mutex> lock(mtx2);
                encoders[0]->encode_video_mp4(Utils::change_frame_size(width, height, av_frame_clone(
                        decoders[0]->get_video_queue().front())));
            }
            per_second_counts++;
            video_frames_counts--;

//            {
//                std::lock_guard<std::mutex> lock(mtx);
                decoders[0]->pop_video();
            //}
        }

        {
            std::unique_lock<std::mutex> lock(mtx);
            cond.notify_all();
        }

    }
    video_over = 1;
    encoders[0]->encode_video_mp4(nullptr);
    return 0;
}







int Transcoder::encode_thread_audio_device_mp4_separate(int& time, int& audio_over) {
    int ret;
    int audio_frames_counts = (samplerate / encoders[0]->get_audio_enc_ctx()->frame_size) * time;

    int64_t per_second_time = Timer::getCurrentTime();
    int per_second_counts = 0;

    while(audio_frames_counts >= 0){
        if(Timer::getCurrentTime() - per_second_counts >= 1000000){
            per_second_counts = 0;
            per_second_time = Timer::getCurrentTime();
        }

        if(per_second_counts >= samplerate * 1.0 / encoders[0]->get_audio_enc_ctx()->frame_size){
            av_usleep(1000);
            continue;
        }


        int fifo_time = 1;
        while(fifo_time--){
            if(!decoders[1]->get_audio_queue().empty()){
                Utils::write_to_fifo(fifo, swr_ctx,
                                     av_frame_clone(decoders[1]->get_audio_queue().front()),
                                     encoders[0]->get_audio_enc_ctx()->channels,
                                     encoders[0]->get_audio_enc_ctx()->sample_fmt,
                                     encoders[0]->get_audio_enc_ctx()->sample_rate);

                decoders[1]->pop_audio();

            }
        }

        if((ret = av_audio_fifo_size(fifo)) >= encoders[0]->get_audio_enc_ctx()->frame_size){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), false);
            if(audio_frame){
                {
                    std::lock_guard<std::mutex> lock(mtx2);
                    ret = encoders[0]->encode_audio_mp4(audio_frame);
                    if(ret < 0) return -1;
                }
                audio_frames_counts--;
                per_second_counts++;

            } else break;
        }
    }
    audio_over = 1;
    encoders[0]->encode_audio_mp4(nullptr);
    return 0;
}






int Transcoder::encode_thread_video_rtmp_black_frames_separate(int &time) {
    int ret;
    AVFrame *black_frame = FrameCreater::create_video_frame();

    int video_frames_counts = framerate * time;
    int per_second_counts = 0;

    int64_t time_per_second = Timer::getCurrentTime();

    while(video_frames_counts >= 0){
        if(Timer::getCurrentTime() - time_per_second >= 1000000){
            av_log(nullptr, AV_LOG_INFO, "per second video frames nums: %d.\n", per_second_counts);
            per_second_counts = 0;
            time_per_second = Timer::getCurrentTime();
        }

        if(per_second_counts >= framerate + 1){
            av_usleep(1000);
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(mtx2);
            encoders[0]->encode_video_rtmp(Utils::change_frame_size(width, height, av_frame_clone(black_frame)));
        }
        per_second_counts++;
        video_frames_counts--;

    }

    encoders[0]->encode_video_rtmp(nullptr);
    av_frame_free(&black_frame);
    av_usleep(7000000);
    return 0;
}


int Transcoder::encode_thread_audio_rtmp_silent_frames_separate(int &time) {
    int ret;
    AVFrame *silent_frame = FrameCreater::create_audio_frame();
    int audio_frames_counts = (encoders[0]->get_audio_enc_ctx()->sample_rate / encoders[0]->get_audio_enc_ctx()->frame_size) * time;
    int per_second_counts = 0;

    int64_t time_per_second = Timer::getCurrentTime();

    while(audio_frames_counts >= 0){
        if(Timer::getCurrentTime() - time_per_second >= 1000000){
            av_log(nullptr, AV_LOG_INFO, "per second audio frames nums: %d.\n", per_second_counts);
            per_second_counts = 0;
            time_per_second = Timer::getCurrentTime();
        }

        if(per_second_counts >= encoders[0]->get_audio_enc_ctx()->sample_rate / encoders[0]->get_audio_enc_ctx()->frame_size){
            av_usleep(1000);
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(mtx2);
            encoders[0]->encode_audio_rtmp(av_frame_clone(silent_frame));
        }
        per_second_counts++;
        audio_frames_counts--;
    }

    encoders[0]->encode_audio_rtmp(nullptr);
    av_frame_free(&silent_frame);
    return 0;
}



int Transcoder::encode_thread_video_mp4_black_frames_separate(int &time) {
    int ret;
    AVFrame *black_frame = FrameCreater::create_video_frame();

    int video_frames_counts = framerate * time;
    while(video_frames_counts >= 0){
        {
            std::lock_guard<std::mutex> lock(mtx2);
            encoders[0]->encode_video_mp4(Utils::change_frame_size(width, height, av_frame_clone(black_frame)));
            video_frames_counts--;
        }
    }
    encoders[0]->encode_video_mp4(nullptr);
    av_frame_free(&black_frame);
    return 0;

}



int Transcoder::encode_thread_audio_mp4_silent_frames_separate(int &time) {
    int ret;
    AVFrame *silent_frame = FrameCreater::create_audio_frame();
    int audio_frames_counts = (encoders[0]->get_audio_enc_ctx()->sample_rate / encoders[0]->get_audio_enc_ctx()->frame_size) * time;
    while(audio_frames_counts >= 0){
        {
            std::lock_guard<std::mutex> lock(mtx2);
            encoders[0]->encode_audio_mp4(av_frame_clone(silent_frame));
            audio_frames_counts--;
        }
    }
    encoders[0]->encode_audio_mp4(nullptr);
    av_frame_free(&silent_frame);
    return 0;

}




int Transcoder::encode_thread_rtsp() {
    unsigned int i;
    int ret;


    std::vector<int> works(inputNums, 1);
    for(i = 0; i < inputNums; i++){
        Thread thread_decode(&Decoder::decode, decoders[i], std::ref(works[i]));
        thread_decode.detach();
    }

    double count_video = 0.0;
    double count_audio = 0.0;


    Thread thread_encode_video(&Transcoder::encode_thread_video_rtsp_no_separate, this, std::ref(works), std::ref(count_video));

    Thread thread_encode_audio(&Transcoder::encode_thread_audio_rtsp_no_separate, this, std::ref(works), std::ref(count_audio));



//    Thread thread_encode_video(&Transcoder::encode_thread_video_rtsp_separate, this, std::ref(works));
//
//    Thread thread_encode_audio(&Transcoder::encode_thread_audio_rtsp_separate, this, std::ref(works));


//    Thread thread_encode_video(&Transcoder::encode_thread_video_rtmp_separate, this, std::ref(works));
//
//    Thread thread_encode_audio(&Transcoder::encode_thread_audio_rtmp_separate, this, std::ref(works));

    return 0;
}




int Transcoder::encode_thread_video_rtsp_separate(std::vector<int> &works) {
    unsigned int i;
    bool video_finish;
    std::vector<AVFrame *> last_frames(inputNums);
    std::vector<AVFrame *> merge_frames(inputNums);
    for(int i = 0; i < last_frames.size(); i++){
        last_frames[i] = FrameCreater::create_video_frame();
        merge_frames[i] = FrameCreater::create_video_frame();
    }


    //int per__count = 0;
    //uint64_t per_second_time = Timer::getCurrentTime();
    uint64_t per_frame_time = Timer::getCurrentTime();


    while(true){
        video_finish = true;
        for(i = 0; i < inputNums; i++){
            video_finish = video_finish && (decoders[i]->get_video_queue().empty()) && (works[i] == 0);
        }
        if(video_finish) break;

//        if(Timer::getCurrentTime() - per_second_time >= 1000000){
//            av_log(nullptr, AV_LOG_INFO, "per second video nums:%d.\n", per_second_count);
//            per_second_count = 0;
//            per_second_time = Timer::getCurrentTime();
//        }
//        if(Timer::getCurrentTime() - per_frame_time >= 1000 / framerate * 1000){
////            av_log(nullptr, AV_LOG_INFO, "per frame video nums:%d.\n", per_second_count);
////            per_second_count
//
//            av_log(nullptr, AV_LOG_INFO, "per video frame time:%dms.\n", (Timer::getCurrentTime() - per_frame_time) / 1000);
//            per_frame_time = Timer::getCurrentTime();
//        } else{
//            av_usleep(1000);
//            continue;
//        }
        while(Timer::getCurrentTime() - per_frame_time < 1000000 / framerate){
            av_usleep(1);
        }

        per_frame_time = Timer::getCurrentTime();
        av_log(nullptr, AV_LOG_INFO, "a new loop video:%dus.\n", per_frame_time);




//        if(per_second_count >= framerate + 1){
//            av_usleep(1000);
//            continue;
//        }

        for(i = 0; i < inputNums; i++){
            if(!decoders[i]->get_video_queue().empty()){
                av_frame_free(&last_frames[i]);
                last_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[i]->pop_video();
                }

                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cond.notify_all();
                }
            } else{
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(last_frames[i]);
            }
        }
        {
            std::lock_guard<std::mutex> lcok(mtx2);
            encoders[0]->encode_video_rtmp(Utils::merge_way(width, height, merge_frames));
            //av_usleep(11000);
            //encoders[0]->encode_video_mp4(Utils::merge_way(width, height, merge_frames));
        }
        //per_second_count++;
    }
    encoders[0]->encode_video_rtmp(nullptr);
    //encoders[0]->encode_video_mp4(nullptr);

    for(int i = 0; i < inputNums; i++){
        av_frame_free(&last_frames[i]);
        av_frame_free(&merge_frames[i]);
    }
    while(!last_frames.empty()){
        last_frames.pop_back();
        merge_frames.pop_back();
    }

    return 0;
}






int Transcoder::encode_thread_audio_rtsp_separate(std::vector<int>& works){
    int ret;
    unsigned int i;

    bool audio_finish;
    bool all_audio_finish;

    //int per_second_count = 0;
    //uint64_t per_second_time = Timer::getCurrentTime();
    uint64_t per_frame_time = Timer::getCurrentTime();


    while(true){
        //判断是否所有音频帧结束
        audio_finish = true;
        for(i = 0; i < inputNums; i++){
            audio_finish = audio_finish && (decoders[i]->get_audio_queue().empty()) && (works[i] == 0);
        }
        if(audio_finish && av_audio_fifo_size(fifo) <= 0) break;



//        if(Timer::getCurrentTime() - per_frame_time >= encoders[0]->get_audio_enc_ctx()->frame_size * 1000000000 / encoders[0]->get_audio_enc_ctx()->sample_rate){
//            av_log(nullptr, AV_LOG_INFO, "per audio frame time:%dms.\n", (Timer::getCurrentTime() - per_frame_time) / 1000);
//            per_frame_time = Timer::getCurrentTime();
//        } else{
//            av_usleep(1000);
//            continue;
//        }

//        if(Timer::getCurrentTime() - per_second_time >= 1000000){
//            av_log(nullptr, AV_LOG_INFO, "per second audio nums:%d.\n", per_second_count);
//            per_second_count = 0;
//            per_second_time = Timer::getCurrentTime();
//        }
//
//        if(per_second_count >= samplerate * 1.0 / encoders[0]->get_audio_enc_ctx()->frame_size){
//            av_usleep(1000);
//            continue;
//        }

        while(Timer::getCurrentTime() - per_frame_time < encoders[0]->get_audio_enc_ctx()->frame_size * 1000000000 / encoders[0]->get_audio_enc_ctx()->sample_rate + 1000){
            av_usleep(1);
        }

        per_frame_time = Timer::getCurrentTime();
        av_log(nullptr, AV_LOG_INFO, "a new loop audio:%dus.\n", per_frame_time);



        //使用计数是为了保证fifo中存足够的音频数据
        int time = 2;
        while(time--){
            for(i = 0; i < inputNums; i++){
                if(i == 0){
                    if(!decoders[i]->get_audio_queue().empty()) {
                        Utils::write_to_fifo(fifo, swr_ctx,
                                             av_frame_clone(decoders[i]->get_audio_queue().front()),
                                             encoders[i]->get_audio_enc_ctx()->channels,
                                             encoders[i]->get_audio_enc_ctx()->sample_fmt,
                                             encoders[i]->get_audio_enc_ctx()->sample_rate);

                        {
                            std::unique_lock<std::mutex> lock(mtx);
                            cond.notify_all();
                        }

                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            decoders[i]->pop_audio();
                        }
                    }
                } else{
                    //因为暂时只用到了第一个视频的音频 所以其余音频帧需要被释放
                    while(!decoders[i]->get_audio_queue().empty()){
                        std::lock_guard<std::mutex> lock(mtx);
                        decoders[i]->pop_audio();
                    }
                }
            }
        }

        all_audio_finish = works[0] == 0 && decoders[0]->get_audio_queue().empty();

        if(av_audio_fifo_size(fifo) >= encoders[0]->get_audio_enc_ctx()->frame_size || (av_audio_fifo_size(fifo) > 0 && all_audio_finish)){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
            if(audio_frame){
                {
                    std::lock_guard<std::mutex> lock(mtx2);
                    ret = encoders[0]->encode_audio_rtmp(audio_frame);
                   // av_usleep(300);
                    //ret = encoders[0]->encode_audio_mp4(audio_frame);
                }
                if(ret < 0) return -1;
                //per_second_count++;
            } else break;
        }

    }
    encoders[0]->encode_audio_rtmp(nullptr);
    //encoders[0]->encode_audio_mp4(nullptr);
    return 0;

}


int Transcoder::encode_thread_video_rtsp_no_separate(std::vector<int> &works, double &count_video) {
    int ret;
    unsigned int i;

    int64_t last_time = -1;
    bool video_finish;
    std::vector<AVFrame *> last_frames(inputNums);
    std::vector<AVFrame *> merge_frames(inputNums);
    for(int i = 0; i < last_frames.size(); i++){
        last_frames[i] = FrameCreater::create_video_frame();
        merge_frames[i] = FrameCreater::create_video_frame();
    }

    while(true){
        video_finish = true;
        for(i = 0; i < works.size(); i++){
            video_finish = video_finish && decoders[0]->get_video_queue().empty() && (works[i] == 0);
        }
        if(video_finish) break;


        for(i = 0; i < inputNums; i++){
            if(!decoders[i]->get_video_queue().empty()){
                av_frame_free(&last_frames[i]);
                last_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(decoders[i]->get_video_queue().front());
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    decoders[i]->pop_video();
                }

                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cond.notify_all();
                }
            } else{
                av_frame_free(&merge_frames[i]);
                merge_frames[i] = av_frame_clone(last_frames[i]);
            }
        }

        encoders[0]->encode_video_rtmp(Utils::merge_way(width, height, merge_frames), count_video, last_time);

//        {
//            std::lock_guard<std::mutex> lock(mtx2);
//            encoders[0]->encode_video_rtmp(Utils::merge_way(width, height, merge_frames), count_video, last_time);
//        }
    }

    encoders[0]->encode_video_rtmp(nullptr);

    for(int i = 0; i < inputNums; i++){
        av_frame_free(&last_frames[i]);
        av_frame_free(&merge_frames[i]);
    }
    while(!last_frames.empty()){
        last_frames.pop_back();
        merge_frames.pop_back();
    }

    return 0;
}




int Transcoder::encode_thread_audio_rtsp_no_separate(std::vector<int> &works, double &count_audio) {
    int ret;
    unsigned int i;
    bool audio_finish;
    bool all_audio_finish;

    int64_t last_time = -1;

    //计算一帧音频播放的时间
    double per_frame_audio = encoders[0]->get_audio_enc_ctx()->frame_size * 1000 * 1.0 / samplerate;

    while(true){

        //判断是否所有音频帧结束
        audio_finish = true;
        for(i = 0; i < inputNums; i++){
            audio_finish = audio_finish && (decoders[i]->get_audio_queue().empty()) && (works[i] == 0);
        }
        if(audio_finish) break;

        //使用计数是为了保证fifo中存足够的音频数据
        int time = 3;
        while(time--){
            for(i = 0; i < inputNums; i++){
                if(i == 0){
                    if(!decoders[i]->get_audio_queue().empty()) {
                        Utils::write_to_fifo(fifo, swr_ctx,
                                             av_frame_clone(decoders[i]->get_audio_queue().front()),
                                             encoders[i]->get_audio_enc_ctx()->channels,
                                             encoders[i]->get_audio_enc_ctx()->sample_fmt,
                                             encoders[i]->get_audio_enc_ctx()->sample_rate);
                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            decoders[i]->pop_audio();
                        }
                    }
                } else{
                    //因为暂时只用到了第一个视频的音频 所以其余音频帧需要被释放
                    while(!decoders[i]->get_audio_queue().empty()){
                        std::lock_guard<std::mutex> lock(mtx);
                        decoders[i]->pop_audio();
                    }
                }
            }
        }

        all_audio_finish = works[0] == 0 && decoders[0]->get_audio_queue().empty();
        //当fifo中有足够数据时 进行编码音频
        while(av_audio_fifo_size(fifo) >= encoders[0]->get_audio_enc_ctx()->frame_size || (av_audio_fifo_size(fifo) > 0 && all_audio_finish)){
            AVFrame *audio_frame = Utils::get_sample_fixed_frame(fifo, encoders[0]->get_audio_enc_ctx(), all_audio_finish);
            if(audio_frame){
                ret = encoders[0]->encode_audio_rtmp(audio_frame, count_audio, last_time);
//                {
//                    std::lock_guard<std::mutex> lock(mtx2);
//                    //ret = encoders[0]->encode_audio(audio_frame);
//                    ret = encoders[0]->encode_audio_rtmp(audio_frame, count_audio, last_time);
//                    //count_audio += per_frame_audio;
//                }
//                {
//                    std::unique_lock<std::mutex> lock(mtx2);
//                    cond2.notify_all();
//                }

                if(ret < 0) return -1;
            } else break;
        }

    }
    encoders[0]->encode_audio(nullptr);
    return 0;
}

