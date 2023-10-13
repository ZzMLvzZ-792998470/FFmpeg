#include "distribute.h"

Distributer::Distributer(std::vector<AVFormatContext *> &ofmt_ctxs) : ofmt_ctxs(ofmt_ctxs) {

}


Distributer::~Distributer() {
    while(!ofmt_ctxs.empty()){
        ofmt_ctxs.pop_back();
    }
}


int Distributer::distribute(AVPacket *pkt) {
    int ret;
    unsigned int i;
    int time = 0;
    for(i = 0; i < ofmt_ctxs.size(); i++){
        if(std::string(ofmt_ctxs[i]->url).substr(0, 3) == "rtp"){
            if(pkt->stream_index == 0){
                if(time == 0) {
                    Thread thread_writePacket(&Writer::write_packets, ofmt_ctxs[i], av_packet_clone(pkt));
                    time = 1;
                } else time = 0;
            } else{
                if(time == 0) time = 1;
                else{
                    time = 0;
                    Thread thread_writePacket(&Writer::write_packets, ofmt_ctxs[i], av_packet_clone(pkt));
                }
            }

        } else Thread thread_writePacket(&Writer::write_packets, ofmt_ctxs[i], av_packet_clone(pkt));
    }

   // av_packet_free(&pkt);
    return 0;
}