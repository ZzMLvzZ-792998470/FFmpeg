#ifndef _THREADSAFEQUEUE_H
#define _THREADSAFEQUEUE_H

#include <deque>
#include <mutex>

template<typename T>
class ThreadSafeDeque {
public:
    ThreadSafeDeque() {}


    void push_front(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        deque_.push_front(std::move(item));
    }

    void push_back(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        deque_.push_back(std::move(item));
    }

    void pop_front() {
        std::lock_guard<std::mutex> lock(mutex_);
        if(!deque_.empty()){
            deque_.pop_front();
        }
    }

    bool pop_back() {
        std::lock_guard<std::mutex> lock(mutex_);
        if(!deque_.empty()){
            deque_.pop_back();
        }
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return deque_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return deque_.size();
    }

    T& front(){
        std::lock_guard<std::mutex> lock(mutex_);
        if(!deque_.empty()) return deque_.front();
    }

    T& back(){
        std::lock_guard<std::mutex> lock(mutex_);
        if(!deque_.empty()) return deque_.back();
    }

    std::deque<T>& get_deque(){
        std::lock_guard<std::mutex> lock(mutex_);
        return deque_;
    }



private:
    std::deque<T> deque_;
    mutable std::mutex mutex_;
};



















#endif
