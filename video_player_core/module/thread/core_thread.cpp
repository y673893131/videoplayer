#include "core_thread.h"

#include "../media/core_media.h"
#include "../util/core_util.h"

core_thread::core_thread()
    :m_media(nullptr)
{

}

core_thread::~core_thread()
{

}

bool core_thread::start(core_media *pInfo)
{
    m_media = pInfo;
    return true;
}

bool core_thread::checkOpt()
{
//    if(testFlag(flag_bit_seek))
//    {
//        msleep(1);
//        return false;
//    }

    if(testFlag(flag_bit_channel_change))
    {
        msleep(1);
       return false;
    }

//    if(testFlag(flag_bit_decode_change))
//    {
//        msleep(1);
//        return false;
//    }

    return true;
}

void core_thread::setFlag(int bit, bool value)
{
    m_media->setFlag(bit, value);
}

bool core_thread::testFlag(int bit)
{
    return m_media->testFlag(bit);
}

bool core_thread::tryPause()
{
    if(testFlag(flag_bit_need_pause))
    {
        m_media->_pause_time = av_gettime();
        setFlag(flag_bit_pause, true);
        setFlag(flag_bit_need_pause, false);
        m_media->_audio->sdl()->pauseSDL();
        m_media->setState(video_player_core::state_paused);
        return true;
    }

    return false;
}

void core_thread::tryStop(int bit)
{
    setFlag(bit);
    if(testFlag(flag_bit_taudio_finish | flag_bit_tvideo_finish | flag_bit_tsubtitle_finish))
        setFlag(flag_bit_Stop);
}
