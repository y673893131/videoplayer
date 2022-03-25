#include "core_thread_audio.h"
#include "../media/core_media.h"
#include "../util/core_util.h"

core_thread_audio* core_thread_audio::s_instance=nullptr;
core_thread_audio::~core_thread_audio()
{
}

core_thread_audio *core_thread_audio::instance()
{
    if(!s_instance)
        s_instance = new core_thread_audio();
    return s_instance;
}

bool core_thread_audio::start(core_media *media)
{
    core_thread::start(media);
    if(!m_media->_audio->isValid())
        return false;
    m_media->_audio->start();
    return true;
}

core_thread_audio::core_thread_audio()
{
}

void core_thread_audio::sdl_audio_call(void *data, Uint8 *stream, int len)
{
//    SDL_SetThreadPriority(SDL_ThreadPriority::SDL_THREAD_PRIORITY_HIGH);
//    ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    auto info = static_cast<core_thread_audio*>(data);
    info->audio_call(stream, static_cast<unsigned int>(len));
}

void core_thread_audio::audio_call(Uint8 *stream, unsigned int len)
{
    auto sdl = m_media->_audio->sdl();
    auto& index = sdl->nBuffIndex;
    auto& size = sdl->nBuffSize;
    auto& buff = sdl->buff;
    unsigned int decodeSize = 0, lenTMP = 0;
    while(len > 0)
    {
        if(testFlag(flag_bit_Stop))
        {
            sdl->pauseSDL();
            break;
        }

        if(index >= size)
        {
            decodeSize = audio_decode();
//#ifdef AUDIO_FILTER
//            Log(Log_Debug, "decode frame length=%d", decodeSize);
//#endif
            if(decodeSize <= 0)
            {
                size = sdl->target.samples;
                memset(buff, 0, size);
            }else
                size = decodeSize;
            index = 0;
        }

        lenTMP = size - index;
        if(lenTMP > len) lenTMP = len;
        if(!buff) return;

        if(testFlag(flag_bit_mute) || testFlag(flag_bit_pause) || testFlag(flag_bit_need_pause) || testFlag(flag_bit_seek))
        {
            memset(buff + index, 0, lenTMP);
            memcpy(stream, reinterpret_cast<uint8_t*>(buff) + index, lenTMP);
        }
        else
        {
            memset(stream, 0, lenTMP);
            sdl->formatChannelType(reinterpret_cast<uint8_t*>(buff) + index, lenTMP, m_media->_cb);
#ifdef AUDIO_WAVE_DISPLAY
            if(m_media->_video->index() < 0)
            {
                sdl->formatFreq(reinterpret_cast<uint8_t*>(buff) + index, lenTMP, m_media->_cb);
            }
#endif
//#ifdef AUDIO_FILTER
//            Log(Log_Debug, "play buff[%u], lenTmp/len=%u/%u", index, lenTMP, size);
//#endif
#ifdef AUDIO_FILTER
            SDL_MixAudio(stream, reinterpret_cast<uint8_t*>(buff) + index, lenTMP, /*sdl->nVol*/SDL_MIX_MAXVOLUME);
#else
            auto vol = sdl->nVol > SDL_MIX_MAXVOLUME ? SDL_MIX_MAXVOLUME : sdl->nVol;
            SDL_MixAudio(stream, reinterpret_cast<uint8_t*>(buff) + index, lenTMP, vol);
#endif
        }

        len -= lenTMP;
        stream += lenTMP;
        index += lenTMP;
    }
}

unsigned int core_thread_audio::audio_decode()
{
    unsigned int buffersize = 0;
    auto& audio = *m_media->_audio;
    auto& pks = m_media->_audio->pkts();
    auto sdl = m_media->_audio->sdl();
    auto& index = m_media->_audio->index();
    auto& vIndex = m_media->_video->index();
    int64_t start_time = m_media->start_time() / 1000;

    bool bSeek = false;
    bool bFlag = false;
    bool bTryAgain = false;
    int64_t seekPts = 0;
    AVPacket pk;
    for(;;)
    {
        if(testFlag(flag_bit_Stop))
        {
//            pks.clear();
            Log(Log_Info, "audio_thread break, set stopped.");
            setFlag(flag_bit_taudio_finish);
            sdl->pauseSDL();
            return buffersize;
        }

        if(testFlag(flag_bit_pause))
            break;

        if(!checkOpt())
            continue;

        if(pks.empty(pk, bSeek))
        {
            if(bSeek)
            {
                audio.flush();
                bFlag = true;
                seekPts = audio.getInteralPts(m_media->_seek_pos + start_time * 1000);
                sdl->resetSpec();
                continue;
            }

            if(testFlag(flag_bit_read_finish) /*&& !isSetBit(*m_flag, flag_bit_seek)*/)
            {
                Log(Log_Info, "audio_thread break, pks is empty and read finish.");
                tryStop(flag_bit_taudio_finish);
                break;
            }

            msleep(1);
            continue;
        }

        if(testFlag(flag_bit_save) && index == pk.stream_index)
        {
            m_media->_save->saveAudio(&pk);
        }

        if(!audio.decode(&pk, bTryAgain))
        {
            av_packet_unref(&pk);
            continue;
        }

        if(bFlag)
        {
            if(seekPts >= audio.clock())
            {
                av_packet_unref(&pk);
                continue;
            }

            bFlag = false;
            if(vIndex < 0)
                setFlag(flag_bit_flush, false);
        }

        if(vIndex < 0)
        {
            m_media->_cb->posChange(audio.displayClock() - start_time);
            m_media->_cb->bitRate(pk.size);
            tryPause();
        }

        audio.play(buffersize);
        av_packet_unref(&pk);
        break;
    }

    return buffersize;
}
