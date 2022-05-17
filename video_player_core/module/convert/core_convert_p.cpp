#include "core_convert_p.h"
#include "video_player_core.h"

core_convertPrivate::core_convertPrivate(core_convert* parent)
    : VP_Data(parent)
    , m_frame(nullptr)
    , m_buffer(nullptr)
    , m_size(0)
    , m_codecCtx(nullptr)
{

}

void core_convertPrivate::initFrame()
{
    unInitFrame();
    m_frame = av_frame_alloc();
}

void core_convertPrivate::unInitFrame()
{
    if(m_frame)
    {
        av_frame_free(&m_frame);
        m_frame = nullptr;
    }
}

void core_convertPrivate::setVolume(int)
{
}

int core_convertPrivate::getVolume()
{
    return 0;
}

void core_convertPrivate::setChannelType(int type)
{
}
