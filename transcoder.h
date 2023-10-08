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


    int init_local_device_transcoder();


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


    int encode_thread_video_deivce_mp4(int& time);

    int encode_thread_audio_device_mp4(int& time);


    int reencode_local_device();

    //录屏
    int encode_device_mp4();

    //推流
    int encode_device_rtmp();

    int encode_files_rtmp_test();


    int encode_thread_video_rtmp_new(std::vector<int>& works, double& count_video, double& count_audio);

    int encode_thread_audio_rtmp_new(std::vector<int>& works, double& count_video, double& count_audio);



    //多线程转码推流视频
    int encode_thread_rtmp();

    //多线程转码为MP4
    int encode_thread_mp4();

    //完全独立线程编码视频mp4
    int encode_thread_video_mp4_separate(std::vector<int>& works);

    //完全独立线程编码音频mp4
    int encode_thread_audio_mp4_separate(std::vector<int>& works);


    //完全独立线程编码视频推流，不依赖音频编码计数和时间 一秒内推流指定个数视频帧
    int encode_thread_video_rtmp_separate(std::vector<int>& works);

    //完全独立线程编码音频推流，不依赖视频编码计数和时间 一秒内推流指定个数音频帧
    int encode_thread_audio_rtmp_separate(std::vector<int>& works);



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


    std::vector<IniterI::ptr> IniterIs;
    //std::vector<const std::shared_ptr<Initer>> IniterIs;
    std::vector<IniterO::ptr> IniterOs;

    std::vector<Decoder::ptr> decoders;
    std::vector<Encoder::ptr> encoders;

};




#endif
