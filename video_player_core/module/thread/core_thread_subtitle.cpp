#include "core_thread_subtitle.h"
#include "../media/core_media.h"
#include "../util/core_util.h"

core_thread_subtitle::core_thread_subtitle()
{
}

core_thread_subtitle::~core_thread_subtitle()
{
}

bool core_thread_subtitle::start(core_media *media)
{
    core_thread::start(media);
    std::thread([&]{ this->threadCall(); }).detach();

    return true;
}

void core_thread_subtitle::threadCall()
{
    Log(Log_Info, "[thread_id]: %u", core_util::getThreadId());
    auto& media = m_media;
    auto& audio = *media->_audio;
    auto& subtitle = *media->_subtitle;
    auto& pks = media->_subtitle->pkts();
    auto& index = subtitle.index();

    bool bStart = false;
    bool bSeek = false;
    bool bFlag = false;
    int64_t seekPos = 0;
    int64_t startBaseTime = 0;
    int64_t displaySubtitleClock = 0;
    int64_t displayAudioClock = 0;
    int64_t start_time = m_media->start_time() / 1000;

    AVPacket pk;
    auto& astream = audio.stream;
    subtitle.subtitleHeader(media->_cb);

    for(;;)
    {
        if(testFlag(flag_bit_Stop))
        {
            pks.clear();
            break;
        }

        if(testFlag(flag_bit_pause) && !testFlag(flag_bit_flush))
        {
            msleep(10);
            continue;
        }

        if(!checkOpt())
            continue;

        if(pks.empty(pk, bSeek))
        {
            if(bSeek)
            {
                m_media->_subtitle->flush();
                bFlag = true;
                seekPos = subtitle.getInteralPts(m_media->_seek_pos + start_time * 1000);
                continue;
            }

            if(testFlag(flag_bit_read_finish))
            {
                Log(Log_Info, "video_thread break, pks is empty and read finish.");
                break;
            }

            msleep(1);
            continue;
        }

        if(!subtitle.check(pk.stream_index))
        {
            av_packet_unref(&pk);
            continue;
        }

#if 1
        if(testFlag(flag_bit_save) && index == pk.stream_index)
        {
            media->_save->saveVideo(&pk);
        }
#endif

        bool bTryagain = false;
        for(;;)
        {
            while(subtitle.decode(&pk, bTryagain))
            {
                displaySubtitleClock = subtitle.displayClock();
                if(!bStart)
                {
                    bStart = true;
                    startBaseTime = displaySubtitleClock;
                }

                // video/audio align
                for(;;)
                {
                    if(testFlag(flag_bit_Stop)) break;
                    if(testFlag(flag_bit_seek)) break;
                    if(astream && !testFlag(flag_bit_taudio_finish))
                    {
                        if(testFlag(flag_bit_read_finish) && audio.pks.empty())
                            break;
                        displayAudioClock = audio.displayClock();
                    }
                    else
                    {
                        displayAudioClock = audio.getDisplayPts(av_gettime() - media->_start_time) + startBaseTime;
                    }

//                    if(testFlag(flag_bit_flush)) break;

                    displaySubtitleClock = subtitle.displayClock();
                    if(displaySubtitleClock <= displayAudioClock) break;
                    if(!testFlag(flag_bit_need_pause))
                    {
                        auto delay = displaySubtitleClock - displayAudioClock;
                        delay = delay > 5 ? 5 : delay;
                        msleep(delay);
                    }
                }

                if(testFlag(flag_bit_Stop)) break;
                if(testFlag(flag_bit_seek)) break;

                if(bFlag)
                {
                    if(seekPos >= subtitle.clock())
                        break;
//                    setFlag(flag_bit_flush, false);
                    bFlag = false;
                }


//                media->_cb->posChange(displayVideoClock - start_time);
                m_media->_cb->bitRate(pk.size);
                subtitle.displayFrame(media->_cb, start_time);
                tryPause();

                break;
            }

            if(!bTryagain)
                break;
        }

        av_packet_unref(&pk);
    }

    tryStop(flag_bit_tsubtitle_finish);
    Log(Log_Info, "finished.");
}
