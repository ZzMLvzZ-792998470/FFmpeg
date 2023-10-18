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

#include "audio_resampler.h"

//
//#include <mutex>
//#include <condition_variable>


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


    int init_transcoder();


    /*
     * 使用的方法
     * */


    int init_local_device_transcoder();

    int reencode_local_device();


    //把解码队列的音频数据写到fifo中
    int add_to_fifo(std::vector<int>& works);

    //清理未使用音频解码队列
    int clear_audio_queues(std::vector<int>& works);

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

    int framerate;
    int samplerate;

    int inputNums;
    int outputNums;

    IniterD::ptr initerD;
    Distributer::ptr distributer;

    AudioResampler::ptr audioResampler;


    std::vector<IniterI::ptr> IniterIs;
    std::vector<IniterO::ptr> IniterOs;

    std::vector<Decoder::ptr> decoders;
    std::vector<Encoder::ptr> encoders;

    std::vector<int> works;

    int video_packet_over = 0;
    int audio_packet_over = 0;

    std::mutex m_mtx;
    std::mutex audio_mtx;
    std::mutex video_mtx;
    std::condition_variable cond;
};




#endif
