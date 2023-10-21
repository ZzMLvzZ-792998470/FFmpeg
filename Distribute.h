#ifndef _DISTRIBUTE_H
#define _DISTRIBUTE_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
};

#include <vector>
#include <memory>
#include <condition_variable>


#include "Writer.h"
#include "Thread.h"



class Distributer{
public:
    typedef std::shared_ptr<Distributer> ptr;
    Distributer(std::vector<AVFormatContext* >& ofmt_ctx);
    ~Distributer();


    int distribute(AVPacket* pkt);

    int start();

    int send(int index);

    int stop();


private:
    std::vector<AVFormatContext* > ofmt_ctxs;
    std::vector<AVPacket *> pkts;
    std::vector<int> fmt_type;
    std::vector<std::mutex> mtxs;

    std::mutex m_mtx;

    bool stopping = false;
};















#endif
