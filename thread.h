#ifndef _THREAD_H
#define _THREAD_H
#include "noncopyable.h"

#include <thread>
#include <functional>



class Thread : Noncopyable{
public:
    Thread() : m_thread(), m_joinable(false) {}

    //使用可变参数模板、forward构造线程以及回调函数
    template<typename Function, typename... Args>
    Thread(Function&& func, Args&&... args) : m_thread(), m_joinable(false) {
        start(std::forward<Function>(func), std::forward<Args>(args)...);
    }

    ~Thread() {
        if (m_joinable) {
            m_thread.join();
        }
    }

    template<typename Function, typename... Args>
    void start(Function&& func, Args&&... args) {
        m_thread = std::thread(std::forward<Function>(func), std::forward<Args>(args)...);
        m_joinable = true;
    }

    void detach() {
        if (m_joinable) {
            m_thread.detach();
            m_joinable = false;
        }
    }

    void join() {
        if (m_joinable) {
            m_thread.join();
            m_joinable = false;
        }
    }

    bool joinable() const {
        return m_joinable;
    }

private:
    std::thread m_thread;
    bool m_joinable;
};


#endif
