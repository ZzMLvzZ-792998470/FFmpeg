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



#include "writer.h"
#include "thread.h"



class Distributer{
public:
    typedef std::shared_ptr<Distributer> ptr;
    Distributer(std::vector<AVFormatContext* >& ofmt_ctx);
    ~Distributer();


    int distribute(AVPacket* pkt);

private:
    std::vector<AVFormatContext* > ofmt_ctxs;
    std::mutex m_mtx;
};















#endif
