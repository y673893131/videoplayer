#include "core_util.h"
#include "../common.h"
#include <sstream>
#include <thread>
#ifndef WIN32
#include <unistd.h>
#include <sys/syscall.h>
#else
#include <Windows.h>
#include <string>
#endif

#include <cstdlib>
#include <string>
#include <codecvt>

unsigned int core_util::getThreadId()
{
#ifdef WIN32
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return static_cast<unsigned int>(std::stoi(ss.str()));
#else
    return pthread_self();
#endif
}

bool core_util::isSetBit(unsigned int flag, int bit)
{
    return ((flag >> bit) & 1) == 1;
}

void core_util::setBit(unsigned int& flag, int bit, bool value/* = true*/)
{
    if(!value)
        flag &= ~(1 << bit);
    else
        flag |= (1 << bit);
}

std::string core_util::toDateTime(time_t tm)
{
    struct tm _tm;
#ifdef WIN32
    localtime_s(&_tm, &tm);
#else
    localtime_r(&tm, &_tm);
#endif
    char stm[128] = {};

    _snprintf(stm, 128, "%04d-%02d-%02d_%02d.%02d.%02d",
              _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday,
              _tm.tm_hour, _tm.tm_min, _tm.tm_sec);

    return stm;
}

std::string core_util::toTime(time_t tm)
{
    auto mSec = (int)(tm % 1000);
    tm /= 1000;
    auto hour = (int)tm / 3600;
    auto min = ((int)tm % 3600) / 60;
    auto sec = (int)tm % 60;
    char stm[128] = {};

    _snprintf(stm, 128, "%02d.%02d.%02d.%03d",
              hour, min, sec, mSec);

    return stm;
}
