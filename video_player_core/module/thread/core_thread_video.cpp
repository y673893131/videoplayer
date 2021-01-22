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
    double pts_video = 0, pts_audio = 0;
    auto& video = *media->video;
    auto& audio = *media->audio;
    auto& pks = media->video->pkts();

    reinterpret_cast<video_interface*>(media->_cb)->setVideoSize(video.width(), video.height());

    bool bStart = false;
    double dStartBase = 0.0;
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

        if(pks.empty(pk))
        {
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

        while(video.decode(&pk))
        {
            pts_video = video.clock();
            if(!bStart)
            {
                bStart = true;
                dStartBase = pts_video;
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
                    pts_audio = audio._clock;
                }
                else
                {
                    pts_audio = CALC_PTS(av_gettime(), media->_start_time) + dStartBase;
                    audio._clock = pts_audio;
                }

                pts_video = video.clock();
                if(pts_video <= pts_audio) break;
                if(!testFlag(flag_bit_need_pause))
                {
                    auto delay = (pts_video - pts_audio) * 1000;
                    delay = delay > 5 ? 5 : delay;
                    msleep(delay);
                }
            }

            if(testFlag(flag_bit_Stop)) break;
            if(testFlag(flag_bit_seek)) break;

            media->_cb->posChange(static_cast<int64_t>(pts_video * 1000));
            video.displayFrame(media->_cb);

            if(testFlag(flag_bit_need_pause))
            {
                media->_pause_time = static_cast<double>(av_gettime());
                setFlag(flag_bit_pause, true);
                setFlag(flag_bit_need_pause, false);
                m_media->setState(video_player_core::state_paused);
            }

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
    if(!m_media->subtitle->check(pkt->stream_index))
        return false;
    int got = -1, ret = 0;
    auto pSubtitle = m_media->subtitle->subtitle();
    ret = avcodec_decode_subtitle2(m_media->subtitle->pCodecContext, pSubtitle, &got, pkt);

    char* subtitleString = nullptr;
    if(got > 0)
    {
        unsigned int number = pSubtitle->num_rects;
        for (unsigned int i = 0; i < number; ++i)
        {
            subtitleString = pSubtitle->rects[i]->ass;
            m_media->_cb->displaySubTitleCall(subtitleString, i);
        }

        avsubtitle_free(pSubtitle);
    }

    return true;
}
