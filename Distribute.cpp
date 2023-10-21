#include "Distribute.h"

Distributer::Distributer(std::vector<AVFormatContext *> &ofmt_ctxs) : ofmt_ctxs(ofmt_ctxs) {
    pkts.resize(ofmt_ctxs.size(), nullptr);
    fmt_type.resize(ofmt_ctxs.size());

    mtxs = std::vector<std::mutex>(ofmt_ctxs.size());
    int time = 0;
    for(int i = 0; i < ofmt_ctxs.size(); i++){
        if(std::string(ofmt_ctxs[i]->oformat->name) == "rtp"){
            fmt_type[i] = time ? 2 : 1;
            time = time ? 0 : 1;

        } else fmt_type[i] = 0;
    }
}


Distributer::~Distributer() {
    while(!ofmt_ctxs.empty()){
        ofmt_ctxs.back() = nullptr;
        ofmt_ctxs.pop_back();
        pkts.pop_back();
        fmt_type.pop_back();
        mtxs.pop_back();
    }

}


int Distributer::distribute(AVPacket *pkt) {
    unsigned int i;

    for (i = 0; i < ofmt_ctxs.size(); i++) {
        std::lock_guard<std::mutex> lock(mtxs[i]);
        pkts[i] = av_packet_clone(pkt);
    }

    bool pkts_over;
    while(true){
        pkts_over = true;
        for(i = 0; i < pkts.size(); i++){
            std::lock_guard<std::mutex> lock(mtxs[i]);
            pkts_over = pkts_over && pkts[i] == nullptr;
        }
        if(pkts_over) break;
    }

    return 0;
}

int Distributer::start(){
    //启动线程
    for(int i = 0; i < ofmt_ctxs.size(); i++){
        Thread thread(&Distributer::send, this, i);
        thread.detach();
    }

    return 0;
}


int Distributer::send(int index){
    while(true){
        std::lock_guard<std::mutex> lock(mtxs[index]);
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (stopping) break;
        }
        if(!pkts[index]) continue;

        if(fmt_type[index] != 0){
           if((pkts[index]->stream_index == 0 && fmt_type[index] == 1) || (pkts[index]->stream_index == 1 && fmt_type[index] == 2))
               Writer::write_packets(ofmt_ctxs[index], pkts[index]);
           else av_packet_free(&pkts[index]);

        }  else Writer::write_packets(ofmt_ctxs[index], pkts[index]);
        pkts[index] = nullptr;
    }
    return 0;
}



int Distributer::stop(){
    std::lock_guard<std::mutex> lock(m_mtx);
    stopping = true;
    return 0;
}
