#include "ThreadSafeQueue.h"




template<typename T>
void ThreadSafeDeque<T>::pop_front() {
    std::lock_guard<std::mutex> lock(mutex_);
    if(!deque_.empty()){
        deque_.pop_front();
    }
}


template<typename T>
void ThreadSafeDeque<T>::pop_back() {
    std::lock_guard<std::mutex> lock(mutex_);
    if(!deque_.empty()){
        deque_.pop_back();
    }
}


template<>
void ThreadSafeDeque<AVFrame*>::pop_front() {
    std::lock_guard<std::mutex> lock(mutex_);
    if(!deque_.empty()){
        av_frame_free(&deque_.front());
        deque_.pop_front();
    }
}


template<>
void ThreadSafeDeque<AVFrame*>::pop_back() {
    std::lock_guard<std::mutex> lock(mutex_);
    if(!deque_.empty()){
        av_frame_free(&deque_.back());
        deque_.pop_back();
    }
}