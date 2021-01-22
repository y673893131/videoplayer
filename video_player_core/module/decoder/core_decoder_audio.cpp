#include "core_decoder_audio.h"
#include "../thread/core_thread_audio.h"
#include "../util/core_util.h"

core_sdl_op* core_decoder_audio::s_sdl = nullptr;

core_decoder_audio::core_decoder_audio()
    :nTempIndex(-1)
{
    if(!s_sdl)
    {
        s_sdl = new core_sdl_op;
    }
}

core_decoder_audio::~core_decoder_audio()
{
    pks.clear();
    uninit();
}

bool core_decoder_audio::init(AVFormatContext* format, int index)
{
    uninit();
    if(!core_decoder::init(format, index))
        return false;
    auto audio = core_thread_audio::instance();
    s_sdl->init(audio->sdl_audio_call, audio);
    if(!s_sdl->initResample(pCodecContext))
        return false;
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
}

int core_decoder_audio::getVol()
{
    return s_sdl->getVol();
}

core_sdl_op *core_decoder_audio::sdl()
{
    return s_sdl;
}

void core_decoder_audio::start()
{
    s_sdl->startSDL();
}

bool core_decoder_audio::decode(AVPacket *pk, unsigned int& bufferSize)
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

    if(pk->pts != AV_NOPTS_VALUE)
    {
        _clock = av_q2d(stream->time_base) * pk->pts;
    }

    int got_frame = 0;
    avcodec_decode_audio4(pCodecContext, frame, &got_frame, pk);
    av_packet_unref(pk);
    if(got_frame)
    {
        sdl()->resampleFrame(frame, bufferSize);
        return true;
    }

    return false;
}
