#include <iostream>


#include "Transcoder.h"



int main() {

    int64_t start_time = Timer::getCurrentTime();
    std::vector<std::string> input_files;
    std::vector<std::string> output_files;


    /*
     * 指定输入文件的url
     *
     * */

    std::string file0 = "C://Users//ZZM//Desktop//素材//网上的素材//蔡徐坤打篮球.mp4";
    //std::string file0 = "C://Users//ZZM//Desktop//素材//test.mp4";
   // std::string file0 = "C://Users//ZZM//Desktop//素材//网上的素材//抖肩舞.mp4";
//    std::string file1 = "C://Users//ZZM//Desktop//素材//hot food.mp4";
//    std::string file2 = "C://Users//ZZM//Desktop//素材//网上的素材//蔡徐坤打篮球.mp4";
//    std::string file3 = "C://Users//ZZM//Desktop//素材//test.mp4";
   // std::string file0 = "C://Users//ZZM//Desktop//素材//wy.mp4";
    //std::string file0 = "C://Users//ZZM//Desktop//素材//test_60.mp4";


    input_files.push_back(file0);
//    input_files.push_back(file1);
//    input_files.push_back(file2);
//    input_files.push_back(file3);



    /*
     * 指定输出文件的url
     *
     * */


//    std::string output_file0 = "rtp://127.0.0.1:65564";
//    std::string output_file1 = "rtp://127.0.0.1:65562";

//     std::string output_file0 = "rtmp://localhost:1935/live/livestream";
//     std::string output_file1 = "output.mp4";
//     std::string output_file1 = "rtsp://localhost:8554/live/livestream";
    std::string output_file2 = "rtp://127.0.0.1:65564";
    std::string output_file3 = "rtp://127.0.0.1:65562";

//     output_files.push_back(output_file0);
//     output_files.push_back(output_file1);
     output_files.push_back(output_file2);
     output_files.push_back(output_file3);
//    output_files.push_back(output_file4);



     /*
      * 生成转码类
      * ① 初始化
      * ② 转码推流操作
      * */

    Transcoder::ptr t(new Transcoder(input_files, output_files, 1280, 720, AV_PIX_FMT_YUV420P, 30, 3, 44100, AV_SAMPLE_FMT_FLTP));
    t->init_transcoder();
    t->transcode();


    av_log(nullptr, AV_LOG_INFO, "whole program time: %dms.\n", (Timer::getCurrentTime() - start_time) / 1000);

    return 0;
}
