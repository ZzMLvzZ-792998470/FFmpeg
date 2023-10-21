#include "Timer.h"


int64_t Timer::getCurrentTime() {
    return av_gettime();
}