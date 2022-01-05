#include "core_decoder_audio.h"
#include "../thread/core_thread_audio.h"
#include "../util/core_util.h"
#include "../filter/core_filter_audio.h"

core_sdl_op* core_decoder_audio::s_sdl = nullptr;

core_decoder_audio::core_decoder_audio()
    :nTempIndex(-1)
{
#ifdef AUDIO_FILTER
    INIT_NEW(&m_filter, core_filter_audio)
#endif
    if(!s_sdl)
    {
        s_sdl = new core_sdl_op;
    }
}

core_decoder_audio::~core_decoder_audio()
{
    pks.clear();
    uninit();
#ifdef AUDIO_FILTER
    SAFE_RELEASE_PTR(&m_filter)
#endif
}

bool core_decoder_audio::init(AVFormatContext* format, int index)
{
    uninit();
    if(!core_decoder::init(format, index))
        return false;
    auto audio = core_thread_audio::instance();
    s_sdl->setCodecContext(pCodecContext);
    s_sdl->init(audio->sdl_audio_call, audio);
    if(!s_sdl->initResample())
        return false;
#ifdef AUDIO_FILTER
    m_filter->init(format->streams[index], pCodecContext);
#endif
    return true;
}

void core_decoder_audio::setIndex(int index)
{
    nTempIndex = nStreamIndex;
    core_decoder::setIndex(index);
    Log(Log_Info, "[thread_id]:%d", core_util::getThreadId());
}

void core_decoder_audio::setVol(int vol)
{
    s_sdl->setVol(vol);
#ifdef AUDIO_FILTER
    reinterpret_cast<core_filter_audio*>(m_filter)->setVol(vol);
#endif
}

int core_decoder_audio::getVol()
{
    return s_sdl->getVol();
}

void core_decoder_audio::setAudioChannel(int type)
{
    s_sdl->setChannelType(type);
}

core_sdl_op *core_decoder_audio::sdl()
{
    return s_sdl;
}

void core_decoder_audio::start()
{
    s_sdl->startSDL();
}

bool core_decoder_audio::decode(AVPacket *pk)
{
    if(nStreamIndex != pk->stream_index)
    {
        if(pk->stream_index != nTempIndex && nTempIndex >= 0)
        {
            return false;
        }
    }
    else
    {
        nTempIndex = nStreamIndex;
    }

#ifdef AUDIO_FILTER
    auto ret = avcodec_send_packet(pCodecContext, pk);
    if (ret < 0) {
        Log(Log_Err, "Error while sending a packet to the decoder");
        return false;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(pCodecContext, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            Log(Log_Err, "Error while receiving a frame from the decoder");
            return false;
        }

        calcClock(pk);

        return true;
    }
#else
    int got_frame = 0;
    avcodec_decode_audio4(pCodecContext, frame, &got_frame, pk);
    if(got_frame)
    {
        calcClock(pk);
        return true;
    }
#endif
    return false;
}

void core_decoder_audio::play(unsigned int& bufferSize)
{
#ifdef AUDIO_FILTER
    auto _frame = m_filter->mix(frame);
    if(_frame)
    {
        sdl()->resampleFrame(_frame, bufferSize);
        av_frame_unref(_frame);
    }
    else
        av_frame_unref(frame);
#else
    sdl()->resampleFrame(frame, bufferSize);
#endif
}
