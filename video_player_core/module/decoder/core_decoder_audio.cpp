#include "core_decoder_audio.h"
#include "../thread/core_thread_audio.h"
#include "../util/core_util.h"
#include "../filter/core_filter_audio.h"
#include "../convert/core_convert_audio.h"
#include "../dev/core_dev.h"

//core_sdl_op* core_decoder_audio::s_sdl = nullptr;

core_decoder_audio::core_decoder_audio()
    :nTempIndex(-1)
{
#ifdef AUDIO_FILTER
    INIT_NEW(&m_filter, core_filter_audio)
#endif
    INIT_NEW(&m_convert, core_convert_audio)
//    if(!s_sdl)
//    {
//        s_sdl = new core_sdl_op;
//    }
}

core_decoder_audio::~core_decoder_audio()
{
    pks.clear();
    uninit();
#ifdef AUDIO_FILTER
    SAFE_RELEASE_PTR(&m_filter)
#endif
    SAFE_RELEASE_PTR(&m_convert)
}

bool core_decoder_audio::init(AVFormatContext* formatCtx, int index)
{
    uninit();
    if(!core_decoder::init(formatCtx, index))
        return false;
    auto audio = core_thread_audio::instance();
    m_convert->dev()->setCallBack(audio->audio_callback, audio);
    m_convert->setContext(pCodecContext);
#ifdef AUDIO_FILTER
    if(!m_filter->init(formatCtx->streams[index], pCodecContext))
        return false;
#else
    if(!m_convert->initResample())
        return false;
#endif

    m_convert->reset();
    return true;
}

void core_decoder_audio::setIndex(int index)
{
    nTempIndex = nStreamIndex;
    core_decoder::setIndex(index);
    Log(Log_Info, "[thread_id]:%u", core_util::getThreadId());
}

void core_decoder_audio::setVol(int vol)
{
    m_convert->setVolume(vol);
//    s_sdl->setVol(vol);
#ifdef AUDIO_FILTER
    reinterpret_cast<core_filter_audio*>(m_filter)->setVol(vol);
#endif
}

int core_decoder_audio::getVol()
{
    return m_convert->getVolume();
//    return s_sdl->getVol();
}

void core_decoder_audio::setAudioChannel(int type)
{
    m_convert->setChannelType(type);
    //    s_sdl->setChannelType(type);
}

core_dev *core_decoder_audio::dev()
{
    return m_convert->dev();
}

void core_decoder_audio::start()
{
    m_convert->start();
}

void core_decoder_audio::pause()
{
    m_convert->pause();
}

bool core_decoder_audio::decode(AVPacket *pk, bool& /*bTryAgain*/)
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

bool core_decoder_audio::change(unsigned int& bufferSize)
{
#ifdef AUDIO_FILTER
    auto _frame = m_filter->mix(frame);
    if(_frame)
    {
        m_convert->convert(_frame);
        bufferSize = m_convert->size();
        av_frame_unref(_frame);
    }
    else
    {
        bufferSize = 0;
        av_frame_unref(frame);
        return false;
    }
#else
    m_convert->convert(frame);
    bufferSize = m_convert->size();
#endif

    return true;
}
