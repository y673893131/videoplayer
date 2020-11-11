#include "video_thread.h"
#include "video_define.h"
#include "Log/Log.h"
#include <video_player_core.h>
#include "audio_thread.h"
#include <thread>
#include <memory>
#ifdef unix
#include <pthread.h>
#endif
std::vector<video_thread*> video_thread::m_threads;
video_thread::video_thread(const _video_info_& info)
{
    m_info = new _video_info_(info);
    m_threads.push_back(this);
    m_index = m_threads.size() - 1;
    Log(Log_Info, "thread begin.%d",std::this_thread::get_id());
}

video_thread::video_thread(const video_thread &)
{
    Log(Log_Info, "thread over.%d",std::this_thread::get_id());
}

video_thread::~video_thread()
{
    Log(Log_Info, "thread exit.%d", std::this_thread::get_id());
    SAFE_RELEASE_PTR(&m_info);
}

void video_thread::start(const _video_info_& arg)
{
    std::thread([arg]()
    {
        auto vthread = new video_thread(arg);
        vthread->startPlay();
    }).detach();
}

video_thread *video_thread::index(int index)
{
    if(m_threads.size() > index)
        return m_threads[index];
    Log(Log_Warning, "index %d is not exist. size is %d", index, m_threads.size());
    return nullptr;
}

void video_thread::play()
{
    setBit(m_info->_flag, flag_bit_Stop, false);
    setBit(m_info->_flag, flag_bit_pause, false);
    setBit(m_info->_flag, flag_bit_need_pause, false);
}

void video_thread::setPause()
{
    Log(Log_Info, "pause: start_time(%.2f)", m_info->_start_time/1000000);
    if(!getPause())
        setBit(m_info->_flag, flag_bit_need_pause);
}

bool video_thread::getPause()
{
    return isSetBit(m_info->_flag, flag_bit_pause);
}

void video_thread::continuePlay()
{
    if(getPause())
    {
        auto t = m_info->_start_time;
        m_info->_start_time += av_gettime() - m_info->_pause_time;
        Log(Log_Info, "continue: start_time(%.2f -> %.2f)", t/1000000, m_info->_pause_time/1000000);
        setBit(m_info->_flag, flag_bit_need_pause, false);
        setBit(m_info->_flag, flag_bit_pause, false);
        m_info->state = video_player_core::state_running;
    }
}

void video_thread::setStop()
{
    setBit(m_info->_flag, flag_bit_Stop);
}

bool video_thread::getStop()
{
    return isSetBit(m_info->_flag, flag_bit_tvideo_finish)
            && isSetBit(m_info->_flag, flag_bit_taudio_finish);
}

void video_thread::seekPos(int64_t ms)
{
    if(!isSetBit(m_info->_flag, flag_bit_seek))
    {
        m_info->_seek_pos = ms * 1000;
        setBit(m_info->_flag, flag_bit_seek);
    }
}

void video_thread::setVol(int nVol)
{
    m_info->audio->sdl->nVol = nVol;
    m_info->audio->sdl->fVolPercent = nVol;
}

void video_thread::setMute(bool bMute)
{
    setBit(m_info->_flag, flag_bit_mute, bMute);
}

int video_thread::state()
{
    return m_info->state;
}

void av_log_call(void* ptr, int level, const char* fmt, va_list vl)
{
    if (level <= AV_LOG_INFO)
    {
        char buffer[1024];
        vsprintf(buffer, fmt, vl);
        Log(Log_Info,"msg : [%d] %s", level, buffer);
    }
}

void video_thread::startPlay()
{
    av_log_set_callback(&av_log_call);
    auto info = m_info;
    Log(Log_Info, "video info:%p vol:%.2f mute:%d", info, info->audio->sdl->fVolPercent, isSetBit(info->_flag, flag_bit_mute));
    int ret = 0, videoIndex = -1, audioIndex = -1;
    if(info->_cb)
        ((video_interface*)info->_cb)->startCall(m_index);
    // allocate context
    info->_format_ctx = avformat_alloc_context();
    auto& pFormatCtx = info->_format_ctx;
    Log(Log_Info, "start open input. thread:%d", std::this_thread::get_id());
    // open file
    if((ret = avformat_open_input(&pFormatCtx, info->src.c_str(), nullptr, nullptr)) < 0)
    {
        char buf[1024] = {};
        av_strerror(ret, buf, 1024);
        Log(Log_Err, "open file failed, %s(%d)=%d,%s",__FUNCTION__, __LINE__, ret, buf);
        goto video_thread_startPlay_end;
    }

    Log(Log_Info, "start find stream info.name[%s]",pFormatCtx->iformat->name);
    // find stream info
    if((ret = avformat_find_stream_info(pFormatCtx, nullptr)) < 0)
    {
        Log(Log_Err, "find stream info failed, %s(%d)=%d",__FUNCTION__, __LINE__, ret);
        goto video_thread_startPlay_end;
    }

    Log(Log_Info, "find stream info over.");

    // find video/audio stream index
    for(size_t i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        Log(Log_Info, "find stream info over.[%d]->%d", i, pFormatCtx->streams[i]->codec->codec_type);

        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            videoIndex = i;
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audioIndex < 0)
            audioIndex = i;
    }

    if(info->_cb)
        ((video_interface*)info->_cb)->totalTime(pFormatCtx->duration);

    info->video->nStreamIndex = videoIndex;
    info->audio->nStreamIndex = audioIndex;
    if(videoIndex < 0 && audioIndex < 0) {
        Log(Log_Err, "find video/audio index is valid!", videoIndex, audioIndex);
        goto video_thread_startPlay_end;
    }

    Log(Log_Info, "video_index(%d),audio_index(%d)", videoIndex, audioIndex);
    Log(Log_Info, "this video total time is:%lld", pFormatCtx->duration);

    // video decode
    if(videoIndex >= 0)
    {
        std::thread([&]{ this->video_decode(); }).detach();
    }

    // audio decode
    if(audioIndex >= 0)
        audio_decode_prepare();
    else
        setBit(info->_flag, flag_bit_taudio_finish);

    decode_loop();
video_thread_startPlay_end:
    Log(Log_Info, "%s finished", __FUNCTION__);
    if(info->_cb)
    {
        auto index = m_index;
        auto cb = (video_interface*)info->_cb;
        SAFE_RELEASE_PTR(&m_threads[index]);
        cb->endCall(index);
        OUT_PUT_LEAK();
    }

    return;
}

void video_thread::video_decode()
{
    auto& info = m_info;
    int nRet = 0;
    double pts_video = 0, pts_audio = 0;
    auto& pks = info->video->pks;
    auto& video = *info->video;
    auto& audio = *info->audio;
    auto& stream = info->_format_ctx->streams[info->video->nStreamIndex];
    AVStream* astream = nullptr;
    if(info->audio->nStreamIndex >= 0)
        astream = info->_format_ctx->streams[info->audio->nStreamIndex];
    auto ctx = stream->codec;
    auto pCodec = avcodec_find_decoder(ctx->codec_id);
    if(!pCodec)
    {
        Log(Log_Err, "video find decoder failed!");
        setBit(info->_flag, flag_bit_tvideo_finish);
        return;
    }

    if((nRet = avcodec_open2(ctx, pCodec, nullptr)) < 0)
    {
        Log(Log_Err, "video open2 decoder failed(%d)!", nRet);
        setBit(info->_flag, flag_bit_tvideo_finish);
        return;
    }

    auto frame = av_frame_alloc();
    auto& scale = *info->yuv;
    scale.setsrcCodec(ctx);
    Log(Log_Info, "video src size (%d,%d)!", ctx->width, ctx->height);
    if(info->_cb)
        ((video_interface*)info->_cb)->setVideoSize(scale.w, scale.h);

    info->state = video_player_core::state_running;
    bool bStart = false;
    double dStartBase = 0.0f;
    AVPacket pk;

    for(;;)
    {
        if(isSetBit(info->_flag, flag_bit_Stop))
        {
            pks.clear();
            break;
        }

        if(isSetBit(info->_flag, flag_bit_pause))
        {
            msleep(10);
            continue;
        }

        if(pks.empty(pk))
        {
            if(isSetBit(info->_flag, flag_bit_read_finish) && !isSetBit(info->_flag, flag_bit_seek))
            {
                Log(Log_Info, "video_thread break, pks is empty and read finish.");
                break;
            }
            else{
                msleep(1);
                continue;
            }
        }

//        pk = pks.pop_front();
        if(!pk.data) continue;
        if(!checkSeek(pk, stream)) continue;
        if((nRet = avcodec_send_packet(ctx, &pk)) != 0)
        {
            Log(Log_Warning, "send packet failed[%d]!", nRet);
            av_packet_unref(&pk);
            continue;
        }

        while(!(nRet = avcodec_receive_frame(ctx, frame)))
        {
            if(pk.dts == AV_NOPTS_VALUE && frame->opaque && *(uint64_t*)frame->opaque != AV_NOPTS_VALUE)
                pts_video = *(uint64_t*)frame->opaque;
            else if(pk.dts != AV_NOPTS_VALUE)
                pts_video = pk.dts;
            else
                pts_video = 0;

            pts_video *= av_q2d(stream->time_base);
            video._clock = pts_video;
            if(isSetBit(info->_flag, flag_bit_vseek_finish))
            {
                if(pts_video < info->_seek_time)
                {
                    av_packet_unref(&pk);
                    continue;
                }else
                    setBit(info->_flag, flag_bit_vseek_finish, false);
            }

            if(!bStart)
            {
                bStart = true;
                dStartBase = pts_video;
            }
            // video/audio align
            for(;;)
            {
                if(isSetBit(info->_flag, flag_bit_Stop)) break;
                if(astream && !isSetBit(info->_flag, flag_bit_taudio_finish))
                {
                    if(isSetBit(info->_flag, flag_bit_read_finish) && audio.pks.empty())
                        break;
                    pts_audio = audio._clock;
                }
                else
                {
                    pts_audio = CALC_PTS(av_gettime(), info->_start_time) + dStartBase;
                    audio._clock = pts_audio;
                }

                pts_video = video._clock;
                if(pts_video <= pts_audio) break;
                if(!isSetBit(info->_flag, flag_bit_need_pause))
                {
                    auto delay = (pts_video - pts_audio) * 1000;
                    delay = delay > 5 ? 5 : delay;
                    msleep(delay);
                    if(isSetBit(info->_flag, flag_bit_vseek_finish))
                        break;
//                        Log(Log_Info, "sleep[%.2f][%.2f].",pts_video, pts_audio);
                }
            }

            if(info->_cb)
            {
                ((video_interface*)info->_cb)->posChange(pts_video * 1000);
                scale.scale(frame, info->_cb);
            }
            if(isSetBit(info->_flag, flag_bit_need_pause))
            {
                info->_pause_time = av_gettime();
                setBit(info->_flag, flag_bit_pause, true);
                setBit(info->_flag, flag_bit_need_pause, false);
                m_info->state = video_player_core::state_paused;
            }
        }

        av_packet_unref(&pk);
    }
video_decode_thread_end:
    if(frame) av_free(frame);
    audio.sdl->pauseSDL();
    setBit(info->_flag, flag_bit_Stop);
    setBit(info->_flag, flag_bit_tvideo_finish);
    Log(Log_Info, "%s finished", __FUNCTION__);
}

bool video_thread::audio_decode_prepare()
{
    int nRet = 0;
    auto& info = m_info;
    auto stream = info->_format_ctx->streams[info->audio->nStreamIndex];
    auto ctx = stream->codec;
    auto codec = avcodec_find_decoder(ctx->codec_id);
    if(!codec)
    {
        info->audio->nStreamIndex = -1;
        Log(Log_Warning, "%s avcodec not find.", __FUNCTION__);
        goto audio_decode_prepare_end;
    }
    if((nRet = avcodec_open2(ctx, codec, nullptr)) < 0)
    {
        Log(Log_Warning, "%s avcodec open2 failed[%d].", __FUNCTION__);
        goto audio_decode_prepare_end;
    }

    if(!audio_thread::instance()->addAudio(m_info))
//    if(!info->audio->sdl->init(ctx, sdl_audio_call, this))
        goto audio_decode_prepare_end;
//    info->audio->sdl->startSDL();
    return true;
audio_decode_prepare_end:
    SAFE_RELEASE_PTR(&info->audio->sdl);
    avcodec_close(ctx);
    setBit(info->_flag, flag_bit_taudio_finish);

    return false;
}

void video_thread::decode_loop()
{
    auto& info = m_info;
    auto& aduio_pks = info->audio->pks;
    auto& video_pks = info->video->pks;

//    std::thread([info]
//    {
//        for(;;)
//        {
//            msleep(1000);
//            if(isSetBit(info->_flag, flag_bit_Stop)) break;
//            if(info->_vRead < 10)
//                Log(Log_Info, "per video:%d/s audio:%d/s", info->_vRead, info->_aRead);
//            info->_vRead = info->_aRead = 0;
//        }
//    }).detach();

    info->_start_time = av_gettime();
    Log(Log_Info, "t_base: %lld", info->_start_time);
    for(;;)
    {
        if(isSetBit(info->_flag, flag_bit_Stop))
            break;
        if(isSetBit(info->_flag, flag_bit_seek))
        {
            seek();
            continue;
        }
        if(aduio_pks.isMax() || video_pks.isMax() || isSetBit(info->_flag, flag_bit_pause))
        {
            msleep(10);
            continue;
        }

//        msleep(1);
        if(!push_frame()) break;
    }

    while(!isSetBit(info->_flag, flag_bit_Stop))
         msleep(100);
    aduio_pks.clear();
    video_pks.clear();

    if(info->audio->sdl)
    {
        info->audio->sdl->pauseSDL();
        msleep(100);
    }

    while(!isSetBit(info->_flag, flag_bit_tvideo_finish)/* || !isSetBit(info->_flag, flag_bit_taudio_finish)*/)
         msleep(100);
}

void video_thread::seek()
{
    auto& info = m_info;
    auto target = info->_seek_pos;
    int sIndex = -1;
    if(info->video->nStreamIndex >= 0)
       sIndex = info->video->nStreamIndex;
    else if(info->audio->nStreamIndex >= 0)
       sIndex = info->audio->nStreamIndex;

    AVRational aVRational = {1, AV_TIME_BASE};
    if(sIndex >= 0)
        target = av_rescale_q(target, aVRational, info->_format_ctx->streams[sIndex]->time_base);
    if(av_seek_frame(info->_format_ctx, sIndex, target, AVSEEK_FLAG_BACKWARD) < 0)
        Log(Log_Warning, "%s:seek failed!", info->_format_ctx->filename);
    else{
        if(info->video->nStreamIndex >= 0)
        {
            info->video->pks.clear();
            info->video->pks.push_back(FFMPEG_SEEK);
        }
        if(info->audio->nStreamIndex >= 0)
        {
            info->audio->pks.clear();
            info->audio->pks.push_back(FFMPEG_SEEK);
        }

        info->_start_time = av_gettime() - info->_seek_pos;
        setBit(info->_flag, flag_bit_read_finish, false);
    }

    setBit(info->_flag, flag_bit_seek, false);
    info->_seek_time = CALC_PTS(info->_seek_pos, 0);
    setBit(info->_flag, flag_bit_vseek_finish);
    setBit(info->_flag, flag_bit_aseek_finish);
    if((isSetBit(info->_flag, flag_bit_pause)))
    {
        setBit(info->_flag, flag_bit_pause, false);
        setBit(info->_flag, flag_bit_need_pause);
        info->_pause_time = av_gettime();
    }
}

bool video_thread::push_frame()
{
    int nRet = 0;
    auto& info = m_info;
    auto& ctx = info->_format_ctx;
    auto& vIndex = info->video->nStreamIndex;
    auto& aIndex = info->audio->nStreamIndex;

    AVPacket pk;
    if((nRet = av_read_frame(ctx, &pk)) < 0)
    {
        if(!isSetBit(info->_flag, flag_bit_read_finish))
        {
            char buf[1024] = {};
            av_strerror(nRet, buf, 1024);
            Log(Log_Warning, "read fram failed,%s, video_pks:%d", buf,info->video->pks.m_packets.size());
        }

        setBit(info->_flag, flag_bit_read_finish);
        if(isSetBit(info->_flag, flag_bit_Stop))
            return false;
        msleep(10);
        return true;
    }

    if(vIndex == pk.stream_index)
    {
        ++info->_vRead;
        info->video->pks.push_back(pk);
    }
    else if(aIndex == pk.stream_index)
    {
        if(isSetBit(info->_flag, flag_bit_taudio_finish))
            av_packet_unref(&pk);
        else
        {
            ++info->_aRead;
            info->audio->pks.push_back(pk);
        }
    } else
        av_packet_unref(&pk);


    return true;
}
