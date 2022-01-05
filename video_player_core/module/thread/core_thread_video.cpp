#include "core_thread_video.h"
#include "../media/core_media.h"
#include "../util/core_util.h"

core_thread_video::core_thread_video()
{
}

core_thread_video::~core_thread_video()
{

}

bool core_thread_video::start(core_media* media)
{
    core_thread::start(media);
    std::thread([&]{ this->threadCall(); }).detach();
    return true;
}

void core_thread_video::threadCall()
{
    Log(Log_Info, "[thread_id]: %d", core_util::getThreadId());
    auto& media = m_media;
    auto& video = *media->_video;
    auto& audio = *media->_audio;
    auto& pks = media->_video->pkts();
    auto& index = video.index();

    reinterpret_cast<video_interface*>(media->_cb)->setVideoSize(video.width(), video.height());

    bool bStart = false;
    bool bSeek = false;
    bool bFlag = false;
    int64_t seekPos = 0;
    int64_t startBaseTime = 0;
    int64_t displayVideoClock = 0;
    int64_t displayAudioClock = 0;
    int64_t start_time = m_media->_format_ctx->start_time / 1000;

    AVPacket pk;
    auto& astream = audio.stream;

    media->setState(video_player_core::state_running);

    for(;;)
    {
        if(testFlag(flag_bit_Stop))
        {
            pks.clear();
            break;
        }

        if(testFlag(flag_bit_pause))
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
                m_media->_video->flush();
                m_media->_subtitle->flush();
                bFlag = true;
                seekPos = video.getInteralPts(m_media->_seek_pos);
            }

            if(testFlag(flag_bit_read_finish))
            {
                Log(Log_Info, "video_thread break, pks is empty and read finish.");
                break;
            }

            msleep(1);
            continue;
        }

        if(checkSubTitle(&pk))
        {
            av_packet_unref(&pk);
            continue;
        }

        if(!video.check(pk.stream_index))
        {
            av_packet_unref(&pk);
            continue;
        }

        if(testFlag(flag_bit_decode_change))
        {
            if(video.changeDecodeType(&pk, media->_decodeType))
                setFlag(flag_bit_decode_change, false);
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
            while(video.decode(&pk, bTryagain))
            {
                displayVideoClock = video.displayClock();
                if(!bStart)
                {
                    bStart = true;
                    startBaseTime = displayVideoClock;
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

                    if(testFlag(flag_bit_flush)) break;

                    displayVideoClock = video.displayClock();
                    if(displayVideoClock <= displayAudioClock) break;
                    if(!testFlag(flag_bit_need_pause))
                    {
                        auto delay = displayVideoClock - displayAudioClock;
                        delay = delay > 5 ? 5 : delay;
                        msleep(delay);
                    }
                }

                if(testFlag(flag_bit_Stop)) break;
                if(testFlag(flag_bit_seek)) break;

                if(bFlag)
                {
                    if(seekPos >= video.clock())
                        break;
                    setFlag(flag_bit_flush, false);
                    bFlag = false;

                }

//                if(video.isIFrame(&pk))
//                {
//                    LogB(Log_Debug, "[video]pts: video:%s -> audio:%s"
//                         , core_util::toTime(displayVideoClock).c_str()
//                         , core_util::toTime(displayAudioClock).c_str()
//                         );
//                }
                media->_cb->posChange(displayVideoClock - start_time);
                video.displayFrame(media->_cb);
                if(testFlag(flag_bit_need_pause))
                {
                    media->_pause_time = av_gettime();
                    setFlag(flag_bit_pause, true);
                    setFlag(flag_bit_need_pause, false);
                    audio.sdl()->pauseSDL();
                    m_media->setState(video_player_core::state_paused);
                }


                break;
            }

            if(!bTryagain)
                break;
        }

        av_packet_unref(&pk);
    }

    audio.sdl()->pauseSDL();
    setFlag(flag_bit_Stop);
    setFlag(flag_bit_tvideo_finish);
    Log(Log_Info, "finished.");
}

bool core_thread_video::checkSubTitle(AVPacket *pkt)
{
    if(!m_media->_subtitle->check(pkt->stream_index))
        return false;
    int got = -1, ret = 0;
    auto pSubtitle = m_media->_subtitle->subtitle();
    ret = avcodec_decode_subtitle2(m_media->_subtitle->pCodecContext, pSubtitle, &got, pkt);

    char* subtitleString = nullptr;
    if(got > 0)
    {
        unsigned int number = pSubtitle->num_rects;
        for (unsigned int i = 0; i < number; ++i)
        {
            auto rc = pSubtitle->rects[i];
            switch (rc->type) {
            case SUBTITLE_ASS:
                subtitleString = rc->ass;
                break;
            case SUBTITLE_TEXT:
                subtitleString = rc->text;
                break;
            }

            m_media->_cb->displaySubTitleCall(subtitleString, i, rc->type);
        }
    }

    if(pSubtitle)
        avsubtitle_free(pSubtitle);
    return true;
}
