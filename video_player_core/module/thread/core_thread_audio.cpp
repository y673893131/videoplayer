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
    if(!m_media->audio->isValid())
        return false;
    m_media->audio->start();
    return true;
}

core_thread_audio::core_thread_audio()
{
}

void core_thread_audio::sdl_audio_call(void *data, Uint8 *stream, int len)
{
    auto info = (core_thread_audio*)data;
    info->audio_call(stream, len);
}

void core_thread_audio::audio_call(Uint8 *stream, unsigned int len)
{
    auto sdl = m_media->audio->sdl();
    auto& index = sdl->nBuffIndex;
    auto& size = sdl->nBuffSize;
    auto& buff = sdl->buff;
    unsigned int decodeSize = 0, lenTMP = 0;

    while(len > 0)
    {
        if(index >= size)
        {
            decodeSize = audio_decode();
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

        if(testFlag(flag_bit_mute) || testFlag(flag_bit_pause))
        {
            memset(buff + index, 0, lenTMP);
            memcpy(stream, reinterpret_cast<uint8_t*>(buff) + index, lenTMP);
        }
        else
        {
            memset(stream, 0, lenTMP);
            SDL_MixAudioFormat(stream, reinterpret_cast<uint8_t*>(buff) + index, AUDIO_S16SYS, lenTMP, sdl->nVol);
        }

        len -= lenTMP;
        stream += lenTMP;
        index += lenTMP;
    }
}

unsigned int core_thread_audio::audio_decode()
{
    unsigned int buffersize = 0;
    auto& pks = m_media->audio->pkts();
    auto sdl = m_media->audio->sdl();
    AVPacket pk;
    for(;;)
    {
        if(testFlag(flag_bit_Stop))
        {
            pks.clear();
            Log(Log_Info, "audio_thread break, set stopped.");
            setFlag(flag_bit_taudio_finish);
            sdl->pauseSDL();
            break;
        }

        if(testFlag(flag_bit_pause))
            break;

        if(!checkOpt())
            continue;

        if(pks.empty(pk))
        {
            if(testFlag(flag_bit_read_finish) /*&& !isSetBit(*m_flag, flag_bit_seek)*/)
            {
                Log(Log_Info, "audio_thread break, pks is empty and read finish.");
                setFlag(flag_bit_taudio_finish);
                break;
            }

            msleep(1);
            continue;
        }

        if(!m_media->audio->decode(&pk, buffersize)) continue;
        break;
    }

    return buffersize;
}
