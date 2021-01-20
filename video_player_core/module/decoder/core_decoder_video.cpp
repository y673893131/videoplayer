#include "core_decoder_video.h"
#include "../util/core_util.h"

core_decoder_video::core_decoder_video()
    :m_convert(nullptr)
{
}

core_decoder_video::~core_decoder_video()
{
    SAFE_RELEASE_PTR(&m_convert)
}

bool core_decoder_video::init(AVFormatContext *format, int index)
{
    uninit();
    if(!core_decoder::init(format, index))
    {
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
    core_decoder::uninit();
    SAFE_RELEASE_PTR(&m_convert)
}

void core_decoder_video::setSize(int width, int height)
{
    m_convert->setSize(width, height);
}

int core_decoder_video::width()
{
    return m_convert->width();
}

int core_decoder_video::height()
{
    return m_convert->height();
}

void core_decoder_video::scale(AVFrame *src, void *cb)
{
    m_convert->scale(src, cb);
}


