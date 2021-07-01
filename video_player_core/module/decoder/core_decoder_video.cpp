#include "core_decoder_video.h"
#include "../util/core_util.h"

core_decoder_video::core_decoder_video()
    :m_convert(nullptr)
    ,m_decodeType(video_player_core::decode_software)
{
}

core_decoder_video::~core_decoder_video()
{
    SAFE_RELEASE_PTR(&m_convert)
}

bool core_decoder_video::init(AVFormatContext *formatCtx, int index)
{
    uninit();

    format = formatCtx;
    nStreamIndex = index;

    bool bRet = false;
    switch (m_decodeType) {
    case video_player_core::decode_cuda:
        bRet = core_decoder_hardware::initCuda(format, nStreamIndex);
        break;
    case video_player_core::decode_qsv:
        bRet = core_decoder_hardware::initQsv(format, nStreamIndex);
        break;
    default:
        bRet = core_decoder::init(format, nStreamIndex);
        break;
    }

    if(!bRet)
    {
//        uninit();
        return false;
    }

    INIT_NEW(&m_convert, core_frame_convert)
    m_convert->setsrcCodec(pCodecContext);

    Log(Log_Info, "thread_id:%d", core_util::getThreadId());
    Log(Log_Info, "video src size (%d,%d)!", pCodecContext->width, pCodecContext->height);

    return true;
}

void core_decoder_video::uninit()
{
    core_decoder_hardware::uninit();
    SAFE_RELEASE_PTR(&m_convert)
}

bool core_decoder_video::decode(AVPacket *pk)
{
    if(m_decodeType != video_player_core::decode_software)
        return core_decoder_hardware::decode(pk);

    auto nRet = avcodec_send_packet(pCodecContext, pk);
    if(nRet != 0 && nRet != AVERROR(EAGAIN))
    {
        Log(Log_Warning, "avcodec_send_packet failed[%d]!", nRet);
        return false;
    }

    nRet = avcodec_receive_frame(pCodecContext, frame);
    if(nRet)
    {
        return false;
    }

    calcClock();
    return true;
}

bool core_decoder_video::checkSeekPkt(AVPacket *pk)
{
    if(m_decodeType != video_player_core::decode_software)
        return core_decoder_hardware::checkSeekPkt(pk);

    if(!m_bFirstSeek && !(pk->flags & AV_PKT_FLAG_KEY))
    {
        return true;
    }

    m_bFirstSeek = true;

    if(!decode(pk))
        return true;

    m_bFirstSeek = false;

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
    if(m_convert)
        m_convert->setSize(width, height);
}

int core_decoder_video::width()
{
    if(m_convert)
        return m_convert->width();
    return 0;
}

int core_decoder_video::height()
{
    if(m_convert)
        return m_convert->height();
    return 0;
}

void core_decoder_video::displayFrame(video_interface *cb)
{
    if(m_convert)
        m_convert->scale(frame, cb);
}
