#ifndef _TRANSCODER_H
#define _TRANSCODER_H

#include "Initer.h"
#include "Decoder.h"
#include "Encoder.h"
#include "Writer.h"
#include "FrameCreater.h"
#include "Timer.h"
#include "Thread.h"
#include "FrameConvert.h"


class Transcoder{
public:
    typedef std::shared_ptr<Transcoder> ptr;
    Transcoder(std::vector<std::string>& input_filenames,
               std::vector<std::string>& output_filenames,
               int width = 1920,
               int height = 1080,
               AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P,
               int framerate = 30,
               uint64_t channel_layout = 3,
               int samplerate = 44100,
               AVSampleFormat sample_fmt = AV_SAMPLE_FMT_FLTP);

    ~Transcoder();


    /*
     * 分开初始化的函数
     * */

    int init_INiterIs();

    int init_Decoders();

    int init_Encoder(bool& initerO_has_inited, bool& encoder_has_inited);

    int init_IniterOs(bool& initerO_has_inited, bool& encoder_has_inited, std::vector<AVFormatContext *>& ofmt_ctxs);

    int init_transcoder();

    //处理音频数据
    int dealing_audio(std::vector<int>& works);

    //处理视频数据
    int dealing_video(std::vector<int>& works);

    //转码操作
    int transcode();

    //切流操作
    int change_input_stream(std::string& filename, int& stream_index);


private:
    std::vector<std::string> input_filenames;
    std::vector<std::string> output_filenames;


    int width;
    int height;

    AVPixelFormat pix_fmt;
    int framerate;
    uint64_t channel_layout;
    int samplerate;
    AVSampleFormat sample_fmt;

    int inputNums;
    int outputNums;


    Distributer::ptr distributer;

    FrameConverter::ptr converter;


    std::vector<IniterI::ptr> IniterIs;
    std::vector<IniterO::ptr> IniterOs;

    std::vector<Decoder::ptr> decoders;
    std::vector<Encoder::ptr> encoders;

    std::vector<int> works;

    int video_packet_over = 0;
    int audio_packet_over = 0;

    bool is_changing = false;

};




#endif
