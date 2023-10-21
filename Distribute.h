#ifndef _DISTRIBUTE_H
#define _DISTRIBUTE_H

extern "C"{
#include <libavformat/avformat.h>
};

#include <vector>
#include <memory>


#include "Writer.h"
#include "Thread.h"



class Distributer{
public:
    typedef std::shared_ptr<Distributer> ptr;
    Distributer(std::vector<AVFormatContext* >& ofmt_ctx);
    ~Distributer();

    //分发接口 传入packet 并复制给私有变量pkts数组
    int distribute(AVPacket* pkt);

    //启动线程
    int start();

    //发送函数 根据不同ofmt_ctxs信息 发送pkt
    int send(int index);

    //停止线程
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
