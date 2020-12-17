#include "audio_thread.h"

_sdl_op_* _ffmpeg_audio_info_::sdl = nullptr;
audio_thread* audio_thread::s_instance=nullptr;
audio_thread::audio_thread()
    :audio(nullptr)
    ,m_flag(nullptr)
{

}

audio_thread::~audio_thread()
{
//    SAFE_RELEASE_PTR(&audio);
}

audio_thread *audio_thread::instance()
{
    if(!s_instance)
        s_instance = new audio_thread();
    return s_instance;
}

bool audio_thread::addAudio(_video_info_* pInfo)
{
    m_info = pInfo;
    // resample: in->out
    m_stream = m_info->_format_ctx->streams[m_info->audio->nStreamIndex];
    if(m_info->audio != audio)
        audio = m_info->audio;
    init();

    if(audio->sdl->initResample(m_stream->codec))
    {
        if(m_flag != &m_info->_flag)
            m_flag = &m_info->_flag;
        audio->sdl->startSDL();
        return true;
    }

    return false;
}

bool audio_thread::init()
{
    return audio->sdl->init(sdl_audio_call, this);
}

void audio_thread::sdl_audio_call(void *data, Uint8 *stream, int len)
{
    auto info = (audio_thread*)data;
    info->audio_call(stream, len);
}

void audio_thread::audio_call(Uint8 *stream, int len)
{
    auto sdl = audio->sdl;
    auto& index = sdl->nBuffIndex;
    auto& size = sdl->nBuffSize;
    auto& buff = sdl->buff;
    int decodeSize = 0, lenTMP = 0;

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

        if(isSetBit(*m_flag, flag_bit_mute) || isSetBit(*m_flag, flag_bit_pause))
        {
            memset(buff + index, 0, lenTMP);
            memcpy(stream, (uint8_t*)buff + index, lenTMP);
        }
        else
        {
            memset(stream, 0, lenTMP);
            SDL_MixAudioFormat(stream, (uint8_t*)buff + index, AUDIO_S16SYS, lenTMP, sdl->nVol);
        }

        len -= lenTMP;
        stream += lenTMP;
        index += lenTMP;
    }
}

int audio_thread::audio_decode()
{
    int buffersize = 0;
    auto& pks = audio->pks;
    auto& clock = audio->_clock;
//    auto& stream = info->_format_ctx->streams[audio->nStreamIndex];
    auto& ctx = m_stream->codec;
    auto& frame = audio->pFrame;
    auto& resample_frame = audio->pFrameReSample;
    auto& sdl = audio->sdl;
    auto& swrCtx = sdl->swrCtx;
    if(!frame)
        frame = av_frame_alloc();
    AVPacket pk;
    for(;;)
    {
        if(isSetBit(*m_flag, flag_bit_Stop))
        {
            pks.clear();
            Log(Log_Info, "audio_thread break, set stopped.");
            setBit(*m_flag, flag_bit_taudio_finish);
            audio->sdl->pauseSDL();
            break;
        }

        if(isSetBit(*m_flag, flag_bit_pause))
            break;

        if(pks.empty(pk))
        {
            if(isSetBit(*m_flag, flag_bit_read_finish) && !isSetBit(*m_flag, flag_bit_seek))
            {
                Log(Log_Info, "audio_thread break, pks is empty and read finish.");
                setBit(*m_flag, flag_bit_taudio_finish);
            }
            break;
        }

        if(pk.pts != AV_NOPTS_VALUE)
            clock = av_q2d(m_stream->time_base) * pk.pts;
        if(!checkSeek(pk, m_stream))
            continue;
        if(isSetBit(*m_flag, flag_bit_aseek_finish))
        {
            if(clock < m_info->_seek_time)
            {
                av_packet_unref(&pk);
                continue;
            }
            setBit(*m_flag, flag_bit_aseek_finish, false);
        }

        int got_frame = 0;
        avcodec_decode_audio4(ctx, frame, &got_frame, &pk);
        av_packet_unref(&pk);
        if(got_frame)
        {
            // resample frame: 44100 2 AV_SAMPLE_FMT_S16
            if(!resample_frame)
                resample_frame = av_frame_alloc();

            if(resample_frame->nb_samples != frame->nb_samples)
            {
                auto delay = swr_get_delay(swrCtx, sdl->out.rate) + frame->nb_samples;
                resample_frame->nb_samples = static_cast<int>(av_rescale_rnd(
                    delay, sdl->out.rate, sdl->in.rate, AV_ROUND_UP));
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
