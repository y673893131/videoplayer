#ifndef CORE_UTIL_H
#define CORE_UTIL_H
#include <string>

enum flag_bit{
    flag_bit_seek = 0,
    flag_bit_pause,
    flag_bit_Stop,
    flag_bit_tvideo_finish,
    flag_bit_taudio_finish,
    flag_bit_tsubtitle_finish,
    flag_bit_mute,
    flag_bit_read_finish,
    flag_bit_need_pause,
    flag_bit_channel_change,
    flag_bit_decode_change,
    flag_bit_save,
    flag_bit_flush,
};

namespace core_util
{
    unsigned int getThreadId();
    bool isSetBit(unsigned int flag, int bit);
    void setBit(unsigned int& flag, int bit, bool value = true);
    std::string toDateTime(time_t tm);
    std::string toTime(time_t tm);
};

#endif // CORE_UTIL_H
