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
    ~Transcoder();

    //初始化转码
    int init_transcoder();

    //转码操作
    int reencode();

    //单文件输出---MP4格式
    int encode_onefile_as_mp4(int& work);

    //单文件输出---rtmp
    int encode_onefile_as_rtmp(int& work);

    //多文件输出---rtmp格式
    int encode_files_as_rtmp();

    //多文件输出---MP4格式
    int encode_files_as_mp4();


    //多线程编码mp4
    int encode_files_mp4_threads();

    //多线程编码推流rtmp
    int encode_files_rtmp_threads();

    //MP4视频编码线程
    int encode_thread_video_mp4(std::vector<int>& works);

    //MP4音频编码线程
    int encode_thread_audio_mp4(std::vector<int>& works);

    //rtmp视频编码线程
    int encode_thread_video_rtmp(std::vector<int>& works, double& count_video, double& count_audio);

    //rtmp音频编码线程
    int encode_thread_audio_rtmp(std::vector<int>& works, double& count_video, double& count_audio);



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


    std::vector<IniterI::ptr> IniterIs;
    std::vector<IniterO::ptr> IniterOs;

    std::vector<Decoder::ptr> decoders;
    std::vector<Encoder::ptr> encoders;

};















#endif
