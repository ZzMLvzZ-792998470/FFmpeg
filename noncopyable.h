#ifndef _NONCOPYABLE_H
#define _NONCOPYABLE_H

class Noncopyable {
public:

    Noncopyable() = default;

    ~Noncopyable() = default;

    Noncopyable(const Noncopyable&) = delete;

    Noncopyable& operator=(const Noncopyable&) = delete;
};


#endif
