#include "initer.h"

Initer::~Initer(){}



IniterI::IniterI(const std::string &filename) : filename(filename) {
}


int IniterI::init_fmt() {
    int ret;

    if ((ret = avformat_open_input(&ifmt_ctx, filename.c_str(), nullptr, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    av_dump_format(ifmt_ctx, 0, filename.c_str(), 0);
    return 0;
}


AVFormatContext* IniterI::get_fmt_ctx() {
    return ifmt_ctx;
}



IniterI::~IniterI(){
    avformat_close_input(&ifmt_ctx);
}


IniterD::IniterD() {
    avdevice_register_all();
//    vfmt_ctx = avformat_alloc_context();
//    afmt_ctx = avformat_alloc_context();
    fmt_ctx = avformat_alloc_context();
}

IniterD::~IniterD() {
    avformat_close_input(&fmt_ctx);
//    avformat_close_input(&afmt_ctx);
//    avformat_close_input(&vfmt_ctx);
}

int IniterD::init_fmt() {
    int ret;

//    AVDictionary *options = NULL;
//    av_dict_set(&options, "rtbufsize", "128M", 0);
//    av_dict_set(&options, "framerate", "25", 0);

//    ret = init_device_vfmt_ctx();
//    if(ret < 0){
//        av_log(nullptr, AV_LOG_ERROR, "init_device_vfmt_ctx failed.\n");
//        return ret;
//    }


    ret = init_device_afmt_ctx();
    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "init_device_afmt_ctx failed.\n");
        return ret;
    }

    av_dump_format(fmt_ctx, 0, "streams-dshow", 0);
    return 0;

}



int IniterD::init_device_vfmt_ctx() {
    video_input_fmt = av_find_input_format("dshow");
    //if (avformat_open_input(&fmt_ctx, "video=dummy", video_input_fmt, nullptr) != 0) {
    if(avformat_open_input(&fmt_ctx, "video=HP Wide Vision HD Camera", video_input_fmt, nullptr) != 0){
        std::cerr << "无法打开视频设备" << std::endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 查找视频流
    int videoStreamIndex = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (videoStreamIndex < 0) {
        std::cerr << "未找到视频流" << std::endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    //av_dump_format(fmt_ctx, 0, "video=HP Wide Vision HD Camera", 0);
    return 0;
}



int IniterD::init_device_afmt_ctx() {
       audio_input_fmt = av_find_input_format("dshow");
   // if (avformat_open_input(&fmt_ctx, "default", audio_input_fmt, nullptr) != 0) {
    if(avformat_open_input(&fmt_ctx, "audio=Microphone Array (Realtek(R) Audio)", audio_input_fmt, nullptr) != 0){
        std::cerr << "无法打开音频设备" << std::endl;
        return -1;
    }

    // 查找音频流
    int audioStreamIndex = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audioStreamIndex < 0) {
        std::cerr << "未找到音频流" << std::endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    //av_dump_format(fmt_ctx, 0, "audio=Microphone Array (Realtek(R) Audio)", 0);
    return 0;

}




AVFormatContext* IniterD::get_fmt_ctx() {
    //return nullptr;
    return fmt_ctx;
}

//AVFormatContext* IniterD::get_afmt_ctx() {
//    return afmt_ctx;
//}
//
//AVFormatContext* IniterD::get_vfmt_ctx() {
//    return vfmt_ctx;
//}



void IniterD::show_devices() {
    const AVInputFormat* inputFormat = av_input_video_device_next(nullptr);
    while (inputFormat != nullptr) {
        printf("Video Device: %s\n", inputFormat->name);
        inputFormat = av_input_video_device_next(inputFormat);
    }

    inputFormat = av_input_audio_device_next(nullptr);
    while (inputFormat != nullptr) {
        printf("Audio Device: %s\n", inputFormat->name);
        inputFormat = av_input_audio_device_next(inputFormat);
    }

}


void IniterD::show_dummy_device_info(const std::string& device_name) {
    AVDictionary* options = nullptr;
    av_dict_set(&options,"list_devices", "true", 0);
    //const AVInputFormat *iformat = av_find_input_format("dshow");
    const AVInputFormat *iformat = av_find_input_format(device_name.c_str());


    std::cout << "Device: " << device_name << "==============\n";
    //printf("Device Info=============\n");
    avformat_open_input(&fmt_ctx, "video=dummy", iformat, &options);
    std::cout << "======================\n";

    //avformat_close_input(&fmt_ctx);
    //printf("========================\n");
}







IniterO::IniterO(const std::string &filename) : filename(filename) {
}

IniterO::~IniterO(){
    avformat_free_context(ofmt_ctx);
}


int IniterO::init_fmt() {
    if(filename.substr(0, 4) == "rtmp")  avformat_alloc_output_context2(&ofmt_ctx, nullptr, "flv", filename.c_str());
    else avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, filename.c_str());
    if (!ofmt_ctx) {
        av_log(nullptr, AV_LOG_ERROR, "Could not create output context\n");
        return AVERROR_UNKNOWN;
    }

    return 0;
}

AVFormatContext* IniterO::get_fmt_ctx() {
    return ofmt_ctx;
}



void IniterO::print_ofmt_info() {
    av_dump_format(ofmt_ctx, 0, filename.c_str(), 1);
}


int IniterO::ofmt_io_open() {
    int ret;

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, filename.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "Could not open output file '%s'", filename.c_str());
            return ret;
        }
    }
    return ret;
}


