#include "core_decoder_video.h"
#include "../util/core_util.h"
#include "../filter/core_filter_video.h"

core_decoder_video::core_decoder_video()
    :m_convert(nullptr)
{
}

core_decoder_video::~core_decoder_video()
{
#ifdef VIDEO_FILTER
    SAFE_RELEASE_PTR(&m_filter)
#else
    SAFE_RELEASE_PTR(&m_convert)
#endif
}

bool core_decoder_video::init(AVFormatContext *formatCtx, int index)
{
    uninit();

    format = formatCtx;
    nStreamIndex = index;

    bool bRet = false;
    switch (m_decodeType) {
    case 0://software
        bRet = core_decoder::init(format, nStreamIndex);
        goto decoder_check;
    default:
        bRet = core_decoder_hardware::init(format, nStreamIndex);
        break;
    }

    if(!bRet)
    {
        uninit();
        setDecodeType(0);
        bRet = core_decoder::init(format, nStreamIndex);
    }

decoder_check:
    if(!bRet)
    {
        uninit();
        Log(Log_Err, "find decoder failed.");
        return false;
    }

#ifdef VIDEO_FILTER
    INIT_NEW(&m_filter, core_filter_video)
    m_filter->init(format->streams[index], pCodecContext);
#else
    INIT_NEW(&m_convert, core_frame_convert)
    m_convert->setsrcCodec(pCodecContext);
#endif
    Log(Log_Info, "thread_id:%d", core_util::getThreadId());
    Log(Log_Info, "video src size (%d,%d)!", pCodecContext->width, pCodecContext->height);

    return true;
}

void core_decoder_video::uninit()
{
    core_decoder_hardware::uninit();
#ifdef VIDEO_FILTER
    SAFE_RELEASE_PTR(&m_filter)
#else
    SAFE_RELEASE_PTR(&m_convert)
#endif
}

bool core_decoder_video::decode(AVPacket *pk, bool& bTryAgain)
{
    if(m_decodeType)
        return core_decoder_hardware::decode(pk, bTryAgain);

    auto nRet = avcodec_send_packet(pCodecContext, pk);
    bTryAgain = (nRet == AVERROR(EAGAIN));
    if(nRet != 0)
    {
        if(!bTryAgain)
        {
            Log(Log_Warning, "avcodec_send_packet failed[%d]!", nRet);
            return false;
        }
    }

    nRet = avcodec_receive_frame(pCodecContext, frame);
    if(nRet)
    {
        if(nRet != AVERROR(EAGAIN))
        {
            Log(Log_Warning, "avcodec_receive_frame failed[%d]!", nRet);
        }
        return false;
    }

    calcClock(pk);
    return true;
}

bool core_decoder_video::checkSeekPkt(AVPacket *pk)
{
    if(!isIFrame(pk))
        return true;

    return false;
}

bool core_decoder_video::changeDecodeType(AVPacket *pk, int type)
{
    if(type == m_decodeType)
        return true;

    if(isIFrame(pk))
    {
        auto oldType = m_decodeType;
        Log(Log_Info, "[%d->%d] begin.", oldType, type);
        setDecodeType(type);
        Log(Log_Info, "[%d->%d] over.", oldType, type);
        return true;
    }

    return false;
}

bool core_decoder_video::setDecodeType(int type)
{
    if(m_decodeType == type)
        return true;

    m_decodeType = type;
    return init(format, nStreamIndex);
}

int core_decoder_video::getDecodeType()
{
    return m_decodeType;
}

void core_decoder_video::setSize(int width, int height)
{
#ifdef VIDEO_FILTER
    dynamic_cast<core_filter_video*>(m_filter)->setScale(width, height);
#else
    if(m_convert)
        m_convert->setSize(width, height);
#endif
}

int core_decoder_video::width()
{
#ifdef VIDEO_FILTER
    return dynamic_cast<core_filter_video*>(m_filter)->width();
#else
    if(m_convert)
        return m_convert->width();
#endif
    return 0;
}

int core_decoder_video::height()
{
#ifdef VIDEO_FILTER
    return dynamic_cast<core_filter_video*>(m_filter)->height();
#else
    if(m_convert)
        return m_convert->height();
#endif
    return 0;
}

void core_decoder_video::displayFrame(video_interface *cb)
{
#ifdef VIDEO_FILTER
    auto _frame = m_filter->mix(frame);
    if(cb)
    {
        auto filter = dynamic_cast<core_filter_video*>(m_filter);
        auto buffer = filter->buffer();
        auto w = filter->width();
        auto h = filter->height();
        cb->displayCall(buffer, w, h);
    }

    av_frame_unref(_frame);
#else
    if(m_convert)
    {

        m_convert->scale(frame, cb);
    }
#endif
}
