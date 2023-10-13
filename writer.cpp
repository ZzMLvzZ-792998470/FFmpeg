#include "writer.h"

std::mutex Writer::mtx;

int Writer::write_header(AVFormatContext* ofmt_ctx) {
    int ret = avformat_write_header(ofmt_ctx, nullptr);
    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "Error occurred when begining(write_header) output file\n");
        return ret;
    }

    return 0;
}


int Writer::write_packets(AVFormatContext* ofmt_ctx, AVPacket *enc_pkt) {
    int ret;

    if(enc_pkt->stream_index == 1) enc_pkt->stream_index = std::string(ofmt_ctx->url).substr(0, 3) == "rtp" ? 0 : 1;
    enc_pkt->pts = enc_pkt->dts = (enc_pkt->pts * ofmt_ctx->streams[enc_pkt->stream_index]->time_base.den / ofmt_ctx->streams[enc_pkt->stream_index]->time_base.num / 1000);

    {
        std::lock_guard<std::mutex> lock(mtx);
        ret = av_interleaved_write_frame(ofmt_ctx, enc_pkt);
    }

    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "Error occurred when during(write_frame) output file\n");
        return ret;
    }
    av_packet_free(&enc_pkt);
    return 0;
}


int Writer::write_tail(AVFormatContext* ofmt_ctx) {
    int ret = av_write_trailer(ofmt_ctx);
    if(ret != 0){
        if(ret < 0){
            av_log(nullptr, AV_LOG_ERROR, "Error occurred when ending(write_tailer) output file\n");
            return ret;
        }
    }
    return 0;
}



int Writer::write_packets_no_interleaved(AVFormatContext *ofmt_ctx, AVPacket *enc_pkt) {
    int ret;
    {
        std::lock_guard<std::mutex> lock(mtx);
        ret = av_write_frame(ofmt_ctx, enc_pkt);
    }
    if(ret < 0){
        av_log(nullptr, AV_LOG_ERROR, "Error occurred when during(write_frame) output file\n");
        return ret;
    }

    return 0;
}
