#include "core_util.h"

#include <sstream>
#include <thread>

int core_util::getThreadId()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return std::stoi(ss.str());
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
