#ifndef CORE_UTIL_H
#define CORE_UTIL_H

enum flag_bit{
    flag_bit_seek = 0,
    flag_bit_pause,
    flag_bit_Stop,
    flag_bit_tvideo_finish,
    flag_bit_taudio_finish,
    flag_bit_mute,
    flag_bit_read_finish,
    flag_bit_need_pause,
    flag_bit_channel_change,
};

namespace core_util
{
    int getThreadId();
    bool isSetBit(unsigned int flag, int bit);
    void setBit(unsigned int& flag, int bit, bool value = true);
};

#endif // CORE_UTIL_H
