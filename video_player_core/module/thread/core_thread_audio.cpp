#include "core_thread_audio.h"
#include "../media/core_media.h"
#include "../util/core_util.h"
#include "../dev/core_dev.h"
#include "../convert/core_convert_audio.h"

core_thread_audio* core_thread_audio::s_instance=nullptr;

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
    : m_nBuffIndex(0)
    , m_nBuffSize(0)
{
}

core_thread_audio::~core_thread_audio()
{
}

void core_thread_audio::audio_callback(void *data, Uint8 *stream, int len)
{
//    SDL_SetThreadPriority(SDL_ThreadPriority::SDL_THREAD_PRIORITY_HIGH);
//    ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    auto info = static_cast<core_thread_audio*>(data);
    info->audio_call(stream, static_cast<unsigned int>(len));
}

void core_thread_audio::audio_call(Uint8 *stream, unsigned int len)
{
    if(!m_media)
        return;
    auto decoder = m_media->_audio;
    if(!decoder) return;
    auto convert = reinterpret_cast<core_convert_audio*>(decoder->convert());
    auto dev = decoder->dev();
    auto buff = convert->buffer();
    auto& index = m_nBuffIndex;
    auto& size = m_nBuffSize;

    unsigned int decodeSize = 0, lenTMP = 0;
    while(len > 0)
    {
        if(testFlag(flag_bit_Stop))
        {
            dev->pause();
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
                size = dev->sample();
                memset(buff, 0, size);
            }else
                size = decodeSize;
            index = 0;
        }

        lenTMP = size - index;
        if(lenTMP > len) lenTMP = len;
        if(!buff) return;

        if(stream) memset(stream, 0, lenTMP);
        if(lenTMP && (testFlag(flag_bit_pause) || testFlag(flag_bit_need_pause) || testFlag(flag_bit_seek)))
        {
            memset(buff + index, 0, lenTMP);
        }
        else
        {
            convert->formatChannelType(buff + index, lenTMP);
#ifdef AUDIO_WAVE_DISPLAY
            if(m_media->_video->index() < 0)
            {
                convert->formatFreq(buff + index, lenTMP, m_media->_cb);
            }
#endif
            if(testFlag(flag_bit_mute))
                memset(buff + index, 0, lenTMP);
//#ifdef AUDIO_FILTER
//            Log(Log_Debug, "play buff[%4u], lenTmp/len=%4u/%4u", index, lenTMP, size);
//#endif
#ifdef AUDIO_FILTER

//            FILE* f = nullptr;
//            fopen_s(&f, "d:/1.pcm", "a+");
//            fwrite(buff + index, lenTMP, 1, f);
//            fclose(f);

            dev->play(stream, buff + index, lenTMP, SDL_MIX_MAXVOLUME);
#else
            dev->play(stream, buff + index, lenTMP, decoder->getVol());
#endif
        }

        len -= lenTMP;
        index += lenTMP;
        if(stream) stream += lenTMP;
    }
}

unsigned int core_thread_audio::audio_decode()
{
    unsigned int buffersize = 0;

    auto decoder    = m_media->_audio;
    auto convert    = reinterpret_cast<core_convert_audio*>(decoder->convert());
    auto dev        = decoder->dev();
    auto& pks       = decoder->pkts();
    auto& index     = decoder->index();
    auto& vIndex    = m_media->_video->index();
    auto start_time = m_media->start_time() / 1000;

    bool bStart = false;
    bool bSeek = false;
    bool bFlag = false;
    bool bTryAgain = false;
    int64_t seekPts = 0;
//    int64_t timePts = 0;
    int64_t startPts = 0;
    int64_t displayPts = 0;
    AVPacket pk;
    for(;;)
    {
        if(testFlag(flag_bit_Stop))
        {
//            pks.clear();
            Log(Log_Info, "audio_thread break, set stopped.");
            setFlag(flag_bit_taudio_finish);
            dev->pause();
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
                decoder->flush();
                bFlag = true;
                seekPts = decoder->getInteralPts(m_media->_seek_pos + start_time * 1000);
                convert->reset();
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

        if(!decoder->decode(&pk, bTryAgain))
        {
            av_packet_unref(&pk);
            continue;
        }

        displayPts = decoder->displayClock();
        if(!bStart)
        {
            bStart = true;
            startPts = displayPts;
        }

        if(bFlag)
        {
            if(seekPts >= decoder->clock())
            {
                av_packet_unref(&pk);
                continue;
            }

            bFlag = false;
            if(vIndex < 0)
                setFlag(flag_bit_flush, false);
        }

        // alignment time
//        for(;;)
//        {
//            if(testFlag(flag_bit_Stop)) break;
//            if(testFlag(flag_bit_seek)) break;
//            timePts = decoder->getDisplayPts(av_gettime() - m_media->_start_time) + startPts;
//            if(testFlag(flag_bit_flush)) break;
//            displayPts = decoder->displayClock();
//            Log(Log_Debug, "displayPts %lld timePts=%lld", displayPts, timePts);
//            if(displayPts <= timePts) break;
//            if(!testFlag(flag_bit_need_pause))
//            {
//                auto delay = displayPts - timePts;
//                delay = delay > 5 ? 5 : delay;
//                msleep(delay);
//                Log(Log_Debug, "delay %lld", delay);
//            }
//        }

        if(vIndex < 0)
        {
            m_media->_cb->posChange(displayPts - start_time);
            m_media->_cb->bitRate(pk.size);
            tryPause();
        }

        av_packet_unref(&pk);
        if(!decoder->change(buffersize))
        {
            continue;
        }

        break;
    }

    return buffersize;
}
