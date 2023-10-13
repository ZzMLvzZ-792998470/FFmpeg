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


    //初始化转码
    int init_transcoder();

    int init_local_device_transcoder();

    //转码操作
    int reencode();

    int reencode_local_device();



    /*
     * 以下大部分为重复的函数...2023/10/12
     * */




    //多线程转码推流rtsp
    int mp4_files2rtsp(int encoder_index);

    //多线程转码推流rtmp
    int mp4_files2rtmp(int encoder_index);

    //多线程转码为MP4
    int mp4files2mp4(int encoder_index);


    int mp4_files2rtp(int encoder_index);




    //录屏
    int device2mp4();

    //推流
    int device2rtmp();




    //单文件输出---MP4格式
    int encode_onefile_as_mp4(int& work);



    //多文件输出---MP4格式
    int encode_files_as_mp4();


    //多线程编码mp4
    int encode_files_mp4_threads();


    //MP4视频编码线程（无依赖）
    int mp4_video_separate(std::vector<int>& works, int& encode_index);

    //MP4音频编码线程（无依赖）
    int mp4_audio_separate(std::vector<int>& works, int& encode_index);




//    //完全独立线程编码视频mp4
//    int mp4_video_separate(std::vector<int>& works);
//
//    //完全独立线程编码音频mp4
//    int mp4_audio_separate(std::vector<int>& works);


    //MP4 编码设备视频
    int mp4_video_deviceSeparate(int& time, int& video_over);

    //MP4 编码设备音频
    int mp4_audio_deviceSeparate(int& time, int& audio_over);



    //MP4 编码黑色帧
    int mp4_video_blackFrameSeparate(int& time);

    //MP4 编码静音帧
    int mp4_audio_silentFrameSeparate(int& time);









    //单文件输出---rtmp
    int encode_onefile_as_rtmp(int& work);

    //多文件输出---rtmp格式
    int encode_files_as_rtmp();


    //多线程编码推流rtmp
    int encode_files_rtmp_threads();


    //rtmp视频编码线程（依赖）
    int encode_thread_video_rtmp(std::vector<int>& works, double& count_video, double& count_audio);

    //rtmp音频编码线程 （依赖）
    int encode_thread_audio_rtmp(std::vector<int>& works, double& count_video, double& count_audio);



    int encode_files_rtmp_test();

    int encode_thread_video_rtmp_new(std::vector<int>& works, double& count_video, double& count_audio);

    int encode_thread_audio_rtmp_new(std::vector<int>& works, double& count_video, double& count_audio);


    //完全独立线程编码视频推流，不依赖音频编码计数和时间 一秒内推流指定个数视频帧
    int rtmp_video_perSecondSeparate(std::vector<int>& works);

    //完全独立线程编码音频推流，不依赖视频编码计数和时间 一秒内推流指定个数音频帧
    int rtmp_audio_perSecondSeparate(std::vector<int>& works);


    //rtmp独立线程推流视频 每个帧指定间隔休眠时间
    int rtmp_video_perFrameSeparate(std::vector<int>& works, int& encode_index);

    //rtmp独立线程推流音频 每个帧指定间隔休眠时间
    int rtmp_audio_perFrameSeparate(std::vector<int>& works, int& encode_index);


    //rtmp 推流设备视频
    int rtmp_video_deviceSeparate(int& time, int& video_over);

    //rtmp 推流设备音频
    int rtmp_audio_deviceSeparate(int& time, int& audio_over);


    //rtmp 推流黑色帧
    int rtmp_video_blackFrameSeparate(int& time);

    //rtmp 推流静音帧
    int rtmp_audio_silentFrameSeparate(int& time);




    //rtsp 视频推流
    int rtsp_video_perFrameSeparate(std::vector<int>& works, int& encode_index);

    //rtsp 音频推流
    int rtsp_audio_perFrameSeparate(std::vector<int>& works, int& encode_index);






    int rtp_video_perFrameSeparate(std::vector<int>& works, int& encode_index);

    int rtp_audio_perFrameSeparate(std::vector<int>& works, int& encode_index);




    int reencode_video(std::vector<int>& works);


    int reencode_audio(std::vector<int>& works);



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

    std::vector<IniterO::ptr> IniterOs;

    std::vector<Decoder::ptr> decoders;
    std::vector<Encoder::ptr> encoders;


    //std::vector<int> packets_over;

    std::vector<std::vector<int> > packets_over;

    //int encoder_type = 0;

};




#endif
