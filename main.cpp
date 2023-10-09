#include <iostream>


#include "transcoder.h"



int main() {

     int64_t start_time = Timer::getCurrentTime();
////    int ret;
//    //std::string file0 = "C://Users//ZZM//Desktop//素材//网上的素材//蔡徐坤打篮球.mp4";
//     // std::string file0 = "C://Users//ZZM//Desktop//素材//test.mp4";
//     //std::string file0 = "C://Users//ZZM//Desktop//素材//网上的素材//抖肩舞.mp4";
////    std::string file1 = "C://Users//ZZM//Desktop//素材//hot food.mp4";
////    std::string file2 = "C://Users//ZZM//Desktop//素材//网上的素材//蔡徐坤打篮球.mp4";
////    std::string file3 = "C://Users//ZZM//Desktop//素材//wy.mp4";
//
//    std::string output_file0 = "rtmp://localhost:1935/live/livestream";
//    //std::string output_file0 = "output.mp4";
////
//     std::vector<std::string> input_files;
//     std::vector<std::string> output_files;
////
//    input_files.push_back(file0);
////    input_files.push_back(file1);
////    input_files.push_back(file2);
////    input_files.push_back(file3);
////
//     output_files.push_back(output_file0);
//
//    Transcoder::ptr t(new Transcoder(input_files, output_files, 1280, 720, 30, 44100));
//    t->init_transcoder();
//    t->reencode();


//    std::vector<std::string> output_files;
//    //std::string output_file0 = "rtmp://localhost:1935/live/livestream";
//     std::string output_file0 = "output.mp4";
//    output_files.push_back(output_file0);
//
//
//    Transcoder::ptr t(new Transcoder(output_files, 640, 360, 25, 44100));
//    t->init_local_device_transcoder();
//    t->reencode_local_device();

    av_log(nullptr, AV_LOG_INFO, "whole program time: %dms.\n", (Timer::getCurrentTime() - start_time) / 1000);


     IniterD::ptr p1(new IniterD());
     //p1->show_dummy_device_info("dshow");
     p1->show_devices();
    return 0;
}
