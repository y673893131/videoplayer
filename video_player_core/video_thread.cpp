#include "video_thread.h"
#include "video_define.h"
#include "Log/Log.h"
#include <video_player_core.h>
#include <thread>
#include <memory>

video_thread::video_thread(std::shared_ptr<_video_info_> info)
{
    m_info = info;
}

video_thread::~video_thread()
{

}

void video_thread::start(_video_info_ *arg)
{
    std::thread([&arg](){
        //auto vthread = std::make_shared<video_thread>();
        auto vthread = new video_thread(std::shared_ptr<_video_info_>(arg));
        vthread->startPlay();
    }).detach();
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
    // define
    auto& info = m_info;
    Log(Log_Info, "video info:%p", m_info.get());
    int ret = 0, videoIndex = -1, audioIndex = -1;

    // allocate context
//    info->_format_ctx = avformat_alloc_context();
    auto& pFormatCtx = info->_format_ctx;
//    if(!pFormatCtx){
//        Log(Log_Err, "alloc context failed, %s(%d)",__FUNCTION__, __LINE__);
//        goto video_thread_startPlay_end;
//    }

    Log(Log_Info, "start open input.");
    // open file
    if((ret = avformat_open_input(&pFormatCtx, info->src.c_str(), nullptr, nullptr)) < 0){
        Log(Log_Err, "open file failed, %s(%d)=%d",__FUNCTION__, __LINE__, ret);
        goto video_thread_startPlay_end;
    }

    Log(Log_Info, "start find stream info.name[%s]",pFormatCtx->iformat->name);
    // find stream info
    if((ret = avformat_find_stream_info(pFormatCtx, nullptr)) < 0){
        Log(Log_Err, "find stream info failed, %s(%d)=%d",__FUNCTION__, __LINE__, ret);
        goto video_thread_startPlay_end;
    }

    Log(Log_Info, "find stream info over.");

    // find video/audio stream index
    for(size_t i = 0; i < pFormatCtx->nb_streams; ++i){
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            videoIndex = i;
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audioIndex < 0)
            audioIndex = i;
    }

    if(info->_cb)
        ((video_interface*)info->_cb)->totalTime(pFormatCtx->duration);

    info->video.nStreamIndex = videoIndex;
    info->audio.nStreamIndex = audioIndex;
    if(videoIndex < 0 && audioIndex < 0) {
        Log(Log_Err, "find video/audio index is valid!", videoIndex, audioIndex);
        goto video_thread_startPlay_end;
    }

    Log(Log_Info, "video_index(%d),audio_index(%d)", videoIndex, audioIndex);
    Log(Log_Info, "this video total time is:%I64d", pFormatCtx->duration);

    // video decode
    if(videoIndex >= 0){
        std::thread([&]{ this->video_decode(); }).detach();
    }

    // audio decode
    if(audioIndex >= 0){
        if(!audio_decode_prepare()){}
//            goto video_thread_startPlay_end;
    }

    decode_loop();
video_thread_startPlay_end:
    Log(Log_Info, "%s finished", __FUNCTION__);
    if(info->_cb)
        ((video_interface*)info->_cb)->endCall();
    return;
}

void video_thread::video_decode()
{
    auto& info = m_info;
    int nRet = 0;
    double pts_video = 0, pts_audio = 0;
    auto& pks = info->video.pks;
    auto& video = info->video;
    auto& audio = info->audio;
    auto& stream = info->_format_ctx->streams[info->video.nStreamIndex];
    AVStream* astream = nullptr;
    if(info->audio.nStreamIndex >= 0)
        astream = info->_format_ctx->streams[info->audio.nStreamIndex];
    auto ctx = stream->codec;
    auto pCodec = avcodec_find_decoder(ctx->codec_id);
    if(!pCodec){
        Log(Log_Err, "video find decoder failed!");
        setBit(info->_flag, flag_bit_tvideo_finish);
        return;
//        goto video_decode_thread_end;
    }

    if((nRet = avcodec_open2(ctx, pCodec, nullptr)) < 0){
        Log(Log_Err, "video open2 decoder failed(%d)!", nRet);
        setBit(info->_flag, flag_bit_tvideo_finish);
        return;
//        goto video_decode_thread_end;
    }

    auto frame = av_frame_alloc();
    auto& scale = info->yuv;
    scale.setsrcCodec(ctx);
    Log(Log_Info, "video src size (%d,%d)!", ctx->width, ctx->height);

    for(;;){
        if(isSetBit(info->_flag, flag_bit_Stop)){
            pks.clear();
            break;
        }

        if(isSetBit(info->_flag, flag_bit_pause)){
            Sleep(10);
            continue;
        }

        if(pks.empty()){
            if(isSetBit(info->_flag, flag_bit_read_finish))
                break;
            else{
                Sleep(1);
                continue;
            }
        }

        auto pk = pks.pop_front();
        if(!checkSeek(pk, stream))
            continue;

        if((nRet = avcodec_send_packet(ctx, &pk)) != 0){
            Log(Log_Warning, "send packet failed[%d]!", nRet);
            av_packet_unref(&pk);
            continue;
        }

        while(!(nRet = avcodec_receive_frame(ctx, frame))){
            if(pk.dts != AV_NOPTS_VALUE)
                pts_video = pk.dts;
            else{
                if(frame->opaque && *(uint64_t*)frame->opaque != AV_NOPTS_VALUE)
                    pts_video = *(uint64_t*)frame->opaque;
                else
                    pts_video = 0;
            }

            pts_video *= av_q2d(stream->time_base);
            video._clock = pts_video;
            if(isSetBit(info->_flag, flag_bit_vseek_finish)){
                if(pts_video < info->_seek_time){
                    av_packet_unref(&pk);
                    continue;
                }else
                    setBit(info->_flag, flag_bit_vseek_finish, false);
            }

            // video/audio align
            for(;;){
                if(isSetBit(info->_flag, flag_bit_Stop)) break;
                if(astream && !isSetBit(info->_flag, flag_bit_taudio_finish)){
                    if(isSetBit(info->_flag, flag_bit_read_finish) && audio.pks.empty())
                        break;
                    pts_audio = audio._clock;
                }
                else{
                    pts_audio = CALC_PTS(av_gettime(), info->_start_time);
                    audio._clock = pts_audio;
                }

                pts_video = video._clock;
                if(pts_video <= pts_audio) break;
                if(!isSetBit(info->_flag, flag_bit_need_pause)){
                    auto delay = (pts_video - pts_audio) * 1000;
                    delay = delay > 5 ? 5 : delay;
                    Sleep(delay);
                }
            }

            scale.scale(frame, info->_cb);
            if(isSetBit(info->_flag, flag_bit_need_pause)){
                setBit(info->_flag, flag_bit_pause, true);
                setBit(info->_flag, flag_bit_need_pause, false);
            }
        }

        av_packet_unref(&pk);
    }
video_decode_thread_end:
    if(frame) av_free(frame);
    setBit(info->_flag, flag_bit_Stop);
    setBit(info->_flag, flag_bit_tvideo_finish);
    Log(Log_Info, "%s finished", __FUNCTION__);
}

bool video_thread::audio_decode_prepare()
{
    int nRet = 0;

    auto& info = m_info;
    auto ctx = info->_format_ctx->streams[info->audio.nStreamIndex]->codec;
    auto codec = avcodec_find_decoder(ctx->codec_id);
    if(!codec){
        info->audio.nStreamIndex = -1;
        Log(Log_Warning, "%s avcodec not find.", __FUNCTION__);
        return true;
    }

    if((nRet = avcodec_open2(ctx, codec, nullptr)) < 0){
        Log(Log_Warning, "%s avcodec open2 failed[%d].", __FUNCTION__);
        return false;
    }

    auto& audio = info->audio;
    // resample: in->out

    _audio_sample_ in(ctx);
    _audio_sample_ out;
    auto swrCtx = swr_alloc_set_opts(nullptr,
                                     out.layout, out.fmt, out.rate,
                                     in.layout, in.fmt, in.rate,
                                     0, nullptr);
    if((nRet = swr_init(swrCtx)) < 0){
        char err[128] = {};
        av_strerror(nRet, err, 128);
        Log(Log_Warning, "open resample failed, %s", err);
        swr_free(&swrCtx);
        avcodec_close(ctx);
        return false;
    }

    auto pStream = info->_format_ctx->streams[audio.nStreamIndex];

    audio.sdl = new _sdl_op_(&out, sdl_audio_call, this);
    auto& sdl = audio.sdl;
    sdl->in = in;
    sdl->out = out;
    sdl->swrCtx = swrCtx;
    if((nRet = sdl->init())){
        Log(Log_Warning, "sdl init failed, %d", nRet);
        goto audio_decode_end;
    }

    if(sdl->open() < 0){
        Log(Log_Warning, "sdl open failed.");
        goto audio_decode_end;
    }

    sdl->start();
    return true;
audio_decode_end:
    if(audio.sdl){
        delete audio.sdl;
        audio.sdl = nullptr;
    }

    avcodec_close(ctx);
    setBit(info->_flag, flag_bit_taudio_finish);
    return false;
}

int video_thread::audio_decode()
{
    int buffersize = 0;
    auto& info = m_info;
    auto& audio = info->audio;
    auto& pks = audio.pks;
    auto& clock = audio._clock;
    auto& stream = info->_format_ctx->streams[audio.nStreamIndex];
    auto& ctx = stream->codec;
    auto& frame = audio.pFrame;
    auto& resample_frame = audio.pFrameReSample;
    auto& sdl = audio.sdl;
    auto& swrCtx = sdl->swrCtx;
    if(!frame)
        frame = av_frame_alloc();
    for(;;){
        if(isSetBit(info->_flag, flag_bit_Stop)){
            setBit(info->_flag, flag_bit_taudio_finish);
            m_info->audio.pks.clear();
            break;
        }

        if(isSetBit(info->_flag, flag_bit_pause))
            break;
        if(pks.empty())
            break;
        auto pk = pks.pop_front();
        if(pk.pts != AV_NOPTS_VALUE)
            clock = av_q2d(stream->time_base) * pk.pts;
        if(!checkSeek(pk, stream))
            continue;
        if(isSetBit(info->_flag, flag_bit_aseek_finish)){
            if(clock < info->_seek_time)
                continue;
            setBit(info->_flag, flag_bit_aseek_finish, false);
        }

        int got_frame = 0;
        auto size = avcodec_decode_audio4(ctx, frame, &got_frame, &pk);
        av_packet_unref(&pk);
        if(got_frame){
            // resample frame: 44100 2 AV_SAMPLE_FMT_S16
            if(!resample_frame)
                resample_frame = av_frame_alloc();

            if(resample_frame->nb_samples != frame->nb_samples){
                auto delay = swr_get_delay(swrCtx, sdl->out.rate) + frame->nb_samples;
                resample_frame->nb_samples = av_rescale_rnd(
                    delay, sdl->out.rate, sdl->in.rate, AV_ROUND_UP);
                av_samples_fill_arrays(resample_frame->data, resample_frame->linesize,
                                       sdl->buff, sdl->target.channels, resample_frame->nb_samples,
                                       sdl->out.fmt, 0);
            }

            auto len = swr_convert(swrCtx, resample_frame->data, resample_frame->nb_samples,
                                   (const uint8_t**)frame->data, frame->nb_samples);
            buffersize = len * sdl->target.channels * av_get_bytes_per_sample(sdl->out.fmt);
            break;
        }
    }

    return buffersize;
}

void video_thread::decode_loop()
{
    auto& info = m_info;
    auto& aduio_pks = info->audio.pks;
    auto& video_pks = info->video.pks;

    info->_start_time = av_gettime();
    for(;;){
        if(isSetBit(info->_flag, flag_bit_Stop))
            break;
        if(isSetBit(info->_flag, flag_bit_seek))
            seek();
        if(aduio_pks.isMax() || video_pks.isMax() || isSetBit(info->_flag, flag_bit_pause)){
            Sleep(10);
            continue;
        }

        if(!push_frame()) break;
    }

    while(!isSetBit(info->_flag, flag_bit_Stop))
        Sleep(100);
    aduio_pks.clear();
    video_pks.clear();

    while(!isSetBit(info->_flag, flag_bit_tvideo_finish)
          || !isSetBit(info->_flag, flag_bit_taudio_finish)){
        Sleep(100);
    }
}

void video_thread::sdl_audio_call(void *data, Uint8 *stream, int len){
    auto info = (video_thread*)data;
    info->audio_call(stream, len);
}

void video_thread::audio_call(Uint8 *stream, int len)
{
    auto& info = m_info;
    auto sdl = info->audio.sdl;
    auto& index = sdl->nBuffIndex;
    auto& size = sdl->nBuffSize;
    auto& buff = sdl->buff;
    int decodeSize = 0, lenTMP = 0;
    while(len > 0){
        if(index >= size){
            decodeSize = audio_decode();
            if(decodeSize <= 0){
                size = sdl->target.samples;
                memset(buff, 0, size);
            }else
                size = decodeSize;
            index = 0;
        }

        lenTMP = size - index;
        if(lenTMP > len) lenTMP = len;
        if(!buff) return;
        if(isSetBit(info->_flag, flag_bit_mute) || isSetBit(info->_flag, flag_bit_pause))
            memset(buff + index, 0, lenTMP);
        else
            sdl->adjustVol(lenTMP);
        memcpy(stream, (uint8_t*)buff + index, lenTMP);
        len -= lenTMP;
        stream += lenTMP;
        index += lenTMP;
    }
}

void video_thread::seek()
{
    auto& info = m_info;
    auto target = info->_seek_pos;
    int sIndex = -1;
    if(info->video.nStreamIndex >= 0)
       sIndex = info->video.nStreamIndex;
    else if(info->audio.nStreamIndex >= 0)
       sIndex = info->audio.nStreamIndex;

    AVRational aVRational = {1, AV_TIME_BASE};
    if(sIndex >= 0)
        target = av_rescale_q(target, aVRational, info->_format_ctx->streams[sIndex]->time_base);
    if(av_seek_frame(info->_format_ctx, sIndex, target, AVSEEK_FLAG_BACKWARD) < 0)
        Log(Log_Warning, "%s:seek failed!", info->_format_ctx->filename);
    else{
        if(info->video.nStreamIndex >= 0){
            info->video.pks.clear();
            info->video.pks.push_back(FFMPEG_SEEK);
        }
        if(info->audio.nStreamIndex >= 0){
            info->audio.pks.clear();
            info->audio.pks.push_back(FFMPEG_SEEK);
        }

        info->_start_time = av_gettime() - info->_seek_pos;
    }

    setBit(info->_flag, flag_bit_seek, false);
    info->_seek_time = CALC_PTS(info->_seek_pos, 0);
    setBit(info->_flag, flag_bit_vseek_finish);
    setBit(info->_flag, flag_bit_aseek_finish);
    if((isSetBit(info->_flag, flag_bit_pause))){
        setBit(info->_flag, flag_bit_pause, false);
        setBit(info->_flag, flag_bit_need_pause);
    }
}

bool video_thread::push_frame()
{
    int nRet = 0;
    auto& info = m_info;
    auto& ctx = info->_format_ctx;
    auto& vIndex = info->video.nStreamIndex;
    auto& aIndex = info->audio.nStreamIndex;

    AVPacket pk;
    if((nRet = av_read_frame(ctx, &pk)) < 0){
        setBit(info->_flag, flag_bit_read_finish);
        if(isSetBit(info->_flag, flag_bit_Stop))
            return false;
        Sleep(10);
        return true;
    }

    if(vIndex == pk.stream_index)
        m_info->video.pks.push_back(pk);
    else if(aIndex == pk.stream_index){
        if(isSetBit(info->_flag, flag_bit_taudio_finish))
            av_packet_unref(&pk);
        else
            m_info->audio.pks.push_back(pk);
    }else
        av_packet_unref(&pk);
    return true;
}

bool video_thread::checkSeek(AVPacket &pk, AVStream *stream)
{
    if(strcmp((char*)pk.data, FFMPEG_SEEK) == 0)
    {
        avcodec_flush_buffers(stream->codec);
        av_packet_unref(&pk);
        return false;
    }

    return true;
}
