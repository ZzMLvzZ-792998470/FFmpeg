#ifndef _WRITER_H
#define _WRITER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
}

#include <string>
#include <memory>
#include <mutex>



class Writer{
public:
    //写输出文件头
    static int write_header(AVFormatContext* ofmt_ctx);

    //写输出包
    static int write_packets(AVFormatContext* ofmt_ctx, AVPacket *enc_pkt);

    //写输出文件尾
    static int write_tail(AVFormatContext* ofmt_ctx);

    //无排列写输出包
    static int write_packets_no_interleaved(AVFormatContext* ofmt_ctx, AVPacket *enc_pkt);

private:
    static std::mutex mtx;
};


#endif
