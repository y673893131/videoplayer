#ifndef CORE_GRAUD_LOCK_H
#define CORE_GRAUD_LOCK_H

#include <mutex>
#include <list>

#define LOCK(x) std::unique_lock<std::mutex> lck(x);

struct core_graud_lock : public std::mutex
{
public:
    core_graud_lock();
    virtual ~core_graud_lock();
};

#endif // CORE_GRAUD_LOCK_H
