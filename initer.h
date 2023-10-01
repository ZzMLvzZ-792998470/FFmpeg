#ifndef _INITER_H
#define _INITER_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
}

#include <string>
#include <memory>


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


class IniterO : Initer{
public:
    typedef std::shared_ptr<IniterO> ptr;
    IniterO(const std::string& filename);
    ~IniterO() override;

    //初始化输出文件
    int init_fmt() override;

    //获取ofmt_ctx
    AVFormatContext* get_fmt_ctx() override;

    //打印信息
    void print_ofmt_info();

    //打开输出文件io
    int ofmt_io_open();


private:
    std::string filename;
    AVFormatContext *ofmt_ctx = nullptr;

};

#endif
