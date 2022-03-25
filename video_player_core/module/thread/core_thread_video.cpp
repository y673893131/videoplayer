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
#ifdef _WIN32
    auto thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, video_entry, this, 0, nullptr));
    ::SetThreadPriority(thread, THREAD_PRIORITY_ABOVE_NORMAL);
    auto pro = ::SetThreadIdealProcessor(thread, MAXIMUM_PROCESSORS);
    ::SetThreadIdealProcessor(thread, pro);
#else
    std::thread([&]{ this->threadCall(); }).detach();
#endif
    return true;
}

unsigned core_thread_video::video_entry(void *p)
{
    auto thread = reinterpret_cast<core_thread_video*>(p);
    thread->threadCall();
    return 0;
}

void core_thread_video::threadCall()
{
    Log(Log_Info, "[thread_id]: %u", core_util::getThreadId());
    auto& media = m_media;
    auto& video = *media->_video;
    auto& audio = *media->_audio;
    auto& pks = media->_video->pkts();
    auto& index = video.index();

    reinterpret_cast<video_interface*>(media->_cb)->setVideoSize(video.width(), video.height());

    bool bStart = false;
    bool bSeek = false;
    bool bSeekIFrame = false;
    bool bSeeking = false;
    int64_t seekPos = 0;
    int64_t seekSeconds = 0;
    int64_t startBaseTime = 0;
    int64_t displayVideoClock = 0;
    int64_t displayAudioClock = 0;
    int64_t start_time = m_media->start_time() / 1000;

    AVPacket pk;
    auto& astream = audio.stream;

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
                m_media->_video->flush();
                bSeekIFrame = false;
                bSeeking = true;
                seekPos = video.getInteralPts(m_media->_seek_pos + start_time * 1000);
                seekSeconds = (seekPos % 10000) / 1000;
                LogB(Log_Debug, "[seek_begin]");
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

        if(!video.check(pk.stream_index))
        {
            av_packet_unref(&pk);
            continue;
        }

        if(!bSeekIFrame)
        {
            if(!video.isIFrame(&pk))
            {
                continue;
            }

            bSeekIFrame = true;
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
//            if(bSeeking) LogB(Log_Debug, "[seek-->] %lld", pk.pts);
            while(video.decode(&pk, bTryagain))
            {
                displayVideoClock = video.displayClock();
//                if(bSeeking) LogB(Log_Debug, "[seek<--] %lld[%d] step[%lld]", displayVideoClock, video.frame->pict_type, abs(seekPos - displayVideoClock));
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

                if(bSeeking)
                {
                    static int Icount = 0;
                    static int Pcount = 0;
                    static int Bcount = 0;
                    switch (video.frame->pict_type) {
                    case AV_PICTURE_TYPE_I:
                        ++Icount;
                        break;
                    case AV_PICTURE_TYPE_P:
                        ++Pcount;
                        break;
                    case AV_PICTURE_TYPE_B:
                        ++Bcount;
                        break;
                    }

                    static int64_t firstPts = displayVideoClock;
                    if(Icount + Pcount + Bcount == 1)
                        firstPts = displayVideoClock;

                    auto curSeconds = (displayVideoClock % 10000) / 1000;//second alignment
                    if(seekPos >= video.clock() && (!(abs(seekPos - displayVideoClock) < 100) || curSeconds != seekSeconds))
                        break;
                    setFlag(flag_bit_flush, false);
                    bSeeking = false;

                    LogB(Log_Debug, "[seek_video]pts: video:%lld/%lld[%llds]->%lld[%llds] [count=[%d,%d,%d]]"
                         , firstPts - start_time
                         , displayVideoClock - start_time
                         , curSeconds
                         , seekPos
                         , seekSeconds
                         , Icount
                         , Pcount
                         , Bcount
                         );
                    Bcount = Pcount = Icount = 0;
                }

//                if(video.isIFrame(&pk))
//                {
//                    LogB(Log_Debug, "[video]pts: video:%s -> audio:%s"
//                         , core_util::toTime(displayVideoClock).c_str()
//                         , core_util::toTime(displayAudioClock).c_str()
//                         );
//                }

                media->_cb->posChange(displayVideoClock - start_time);
                media->_cb->bitRate(pk.size);
                video.displayFrame(media->_cb);

                tryPause();

                break;
            }

            if(!bTryagain)
                break;
        }

        av_packet_unref(&pk);
    }

    audio.sdl()->pauseSDL();
    tryStop(flag_bit_tvideo_finish);
    Log(Log_Info, "finished.");
}
