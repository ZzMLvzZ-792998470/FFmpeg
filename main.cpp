#include <iostream>

#include "transcoder.h"

int main() {
    int ret;
    //std::string file0 = "C://Users//ZZM//Desktop//素材//test.mp4";
     std::string file0 = "C://Users//ZZM//Desktop//素材//网上的素材//抖肩舞.mp4";
    std::string file1 = "C://Users//ZZM//Desktop//素材//hot food.mp4";
    std::string file2 = "C://Users//ZZM//Desktop//素材//网上的素材//蔡徐坤打篮球.mp4";
    std::string file3 = "C://Users//ZZM//Desktop//素材//wy.mp4";


    std::string output_file0 = "rtmp://localhost:1935/live/livestream";
    //std::string output_file0 = "output.mp4";

    std::vector<std::string> input_files;
    std::vector<std::string> output_files;

    input_files.push_back(file0);
    input_files.push_back(file1);
    input_files.push_back(file2);
    input_files.push_back(file3);

    output_files.push_back(output_file0);
    //output_files.push_back(output_file1);

    Transcoder::ptr t(new Transcoder(input_files, output_files, 1920, 1080, 30, 44100));
    t->init_transcoder();
    t->reencode();

    return 0;
}
