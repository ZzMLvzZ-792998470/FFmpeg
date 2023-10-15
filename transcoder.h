#ifndef _TRANSCODER_H
#define _TRANSCODER_H

#include "initer.h"
#include "decoder.h"
#include "encoder.h"
#include "writer.h"
#include "frame_create.h"
#include "utils.h"
#include "timer.h"
#include "thread.h"


#include <mutex>
#include <condition_variable>


class Transcoder{
public:
    typedef std::shared_ptr<Transcoder> ptr;
    Transcoder(std::vector<std::string>& input_filenames,
               std::vector<std::string>& output_filenames,
               int width = 1920,
               int height = 1080,
               int framerate = 30,
               int samplerate = 44100);

    Transcoder(std::vector<std::string>& output_filenames, int width = 1920, int height = 1080, int framerate = 20, int samplerate = 44100);


    ~Transcoder();


    /*
     * 分开初始化的函数
     * */

    int init_INiterIs();

    int init_Decoders();

    int init_Encoder(bool& initerO_has_inited, bool& encoder_has_inited);

    int init_IniterOs(bool& initerO_has_inited, bool& encoder_has_inited, std::vector<AVFormatContext *>& ofmt_ctxs);

    int init_Fifo();


    int test_init_transcoder();


    /*
     * 使用的方法
     * */

    //初始化转码
    int init_transcoder();

    int init_local_device_transcoder();

    //转码操作
    int reencode_old();

    int reencode_local_device();


    int add_to_fifo(std::vector<int>& works);

    int get_from_fifo(std::vector<int>& works);


    int dealing_audio(std::vector<int>& works);


    int dealing_video(std::vector<int>& works);


    //with fifo
    int dealing_audio_old(std::vector<int>& works);


    int transcode();

private:
    std::vector<std::string> input_filenames;
    std::vector<std::string> output_filenames;


    int width;
    int height;

    int framerate;
    int samplerate;

    int inputNums;
    int outputNums;


    SwrContext *swr_ctx = nullptr;
    AVAudioFifo *fifo = nullptr;
    IniterD::ptr initerD;
    Distributer::ptr distributer;


    std::vector<IniterI::ptr> IniterIs;
    std::vector<IniterO::ptr> IniterOs;

    std::vector<Decoder::ptr> decoders;
    std::vector<Encoder::ptr> encoders;


    int video_packet_over = 0;
    int audio_packet_over = 0;

};




#endif
