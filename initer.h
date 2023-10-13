#ifndef _INITER_H
#define _INITER_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavdevice/avdevice.h>
}

#include <string>
#include <memory>
#include <iostream>

//#include "local_device.h"


class Initer{
public:
    typedef std::shared_ptr<Initer> ptr;

    Initer(){};
    virtual ~Initer() = 0;

    virtual int init_fmt() = 0;

    virtual AVFormatContext* get_fmt_ctx() = 0;


private:
    std::string filename;
    AVFormatContext *fmt_ctx;

};


class IniterI : Initer{
public:
    typedef std::shared_ptr<IniterI> ptr;
    IniterI(const std::string& filename);

    ~IniterI() override;

    //初始化输入文件
    int init_fmt() override;

    //获取ifmt_ctx
    AVFormatContext* get_fmt_ctx() override;

private:
    std::string filename;
    AVFormatContext *ifmt_ctx = nullptr;

};


class IniterD : Initer{
public:
    typedef std::shared_ptr<IniterD> ptr;
    IniterD();
    ~IniterD() override;

    int init_fmt() override;


   AVFormatContext *get_fmt_ctx() override;

    void show_devices();

    void show_dummy_device_info(const std::string& device_name);


    int init_device_vfmt_ctx();

    int init_device_afmt_ctx();

    AVFormatContext *get_vfmt_ctx() ;
    AVFormatContext *get_afmt_ctx() ;




private:
    AVFormatContext *fmt_ctx;

    AVFormatContext *vfmt_ctx;
    AVFormatContext *afmt_ctx;

    const AVInputFormat *audio_input_fmt = nullptr;
    const AVInputFormat *video_input_fmt = nullptr;
};



class IniterO : Initer{
public:
    typedef std::shared_ptr<IniterO> ptr;
    IniterO(const std::string& filename);
    ~IniterO() override;

    //初始化输出文件
    int init_fmt() override;

    //获取ofmt_ctx
    AVFormatContext* get_fmt_ctx() override;

    //生成ofmt流
    int ofmt_create_stream(AVCodecContext* enc_ctx);

    //打印信息
    void ofmt_print_info();

    //打开输出文件io
    int ofmt_io_open();


private:
    std::string filename;
    AVFormatContext *ofmt_ctx = nullptr;

};

#endif
