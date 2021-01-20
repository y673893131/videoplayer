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
    int nRet = 0;
    double pts_video = 0, pts_audio = 0;
    auto& video = *media->video;
    auto& audio = *media->audio;
    auto& pks = media->video->pkts();

    reinterpret_cast<video_interface*>(media->_cb)->setVideoSize(video.width(), video.height());

    bool bStart = false;
    double dStartBase = 0.0;
    AVPacket pk;

    auto& astream = audio.stream;
    auto& ctx = video.pCodecContext;
    auto& frame = video.frame;

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

        if(testFlag(flag_bit_seek))
        {
            msleep(1);
            continue;
        }

        if(testFlag(flag_bit_channel_change))
        {
            msleep(1);
            continue;
        }

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

        if(!pk.data)
        {
            av_packet_unref(&pk);
            continue;
        }

        if(checkSubTitle(&pk)) continue;

        if(!video.check(pk.stream_index))
        {
            av_packet_unref(&pk);
            continue;
        }

        nRet = avcodec_send_packet(ctx, &pk);
        if(nRet != 0 && nRet != AVERROR(EAGAIN))
        {
            Log(Log_Warning, "send packet failed[%d]!", nRet);
            av_packet_unref(&pk);
            continue;
        }

        while(!(nRet = avcodec_receive_frame(ctx, frame)))
        {
            pts_video = get_video_pts(frame);
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

                pts_video = video._clock;
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

            reinterpret_cast<video_interface*>(media->_cb)->posChange(static_cast<int64_t>(pts_video * 1000));
            video.scale(frame, media->_cb);

            if(testFlag(flag_bit_need_pause))
            {
                media->_pause_time = static_cast<double>(av_gettime());
                setFlag(flag_bit_pause, true);
                setFlag(flag_bit_need_pause, false);
                m_media->setState(video_player_core::state_paused);
            }
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
        int number = pSubtitle->num_rects;
        for (int i=0; i<number; ++i)
        {
            auto rect = pSubtitle->rects[i];
            subtitleString = pSubtitle->rects[i]->ass;
            reinterpret_cast<video_interface*>(m_media->_cb)->displaySubTitleCall(subtitleString, i);
        }

        avsubtitle_free(pSubtitle);
    }

    av_packet_unref(pkt);
    return true;
}

double core_thread_video::get_video_pts(AVFrame *frame)
{
    double pts_video = 0.0;
//            if(pk.dts == AV_NOPTS_VALUE && frame->opaque && *(uint64_t*)frame->opaque != AV_NOPTS_VALUE)
//                pts_video = *reinterpret_cast<double*>(frame->opaque);
//            else if(pk.dts != AV_NOPTS_VALUE)
//                pts_video = static_cast<double>(pk.dts);
//            else
//                pts_video = 0;

    if(frame->pts != AV_NOPTS_VALUE)
        pts_video = frame->pts;
    else if(frame->pkt_dts != AV_NOPTS_VALUE)
        pts_video = frame->pkt_dts;
    else
    {
        auto opa = *reinterpret_cast<double*>(frame->opaque);
        if(opa != AV_NOPTS_VALUE)
            pts_video = opa;
        else
            pts_video = 0;
    }

    auto& video = *m_media->video;
    auto& stream = m_media->_format_ctx->streams[m_media->video->nStreamIndex];

    pts_video *= av_q2d(stream->time_base);
    video._clock = pts_video;

    return pts_video;
}
