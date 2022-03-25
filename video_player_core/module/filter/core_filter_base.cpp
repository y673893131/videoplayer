#include "core_filter_base.h"
#include "core_filter_private.h"
#include "../util/core_util.h"
#include "../media/core_media.h"
#include "../common.h"
#include "Log/Log.h"

core_filter_base::core_filter_base()
    : m_private(nullptr)
    , m_flag(0)
    , m_bInit(false)
{
    INIT_NEW(&m_private, core_filter_private)
}

core_filter_base::~core_filter_base()
{
    SAFE_RELEASE_PTR(&m_private)
}

bool core_filter_base::init(AVStream* stream, AVCodecContext* pCodecContext)
{
    uninit();
    avfilter_register_all();

    if(!m_private->init(stream, pCodecContext))
        return false;

    if(!initParam(stream, pCodecContext))
        return false;

    if(!initFilter())
        return false;

    if(!m_private->link())
        return false;

    m_bInit = true;
    return true;
}

void core_filter_base::uninit()
{
    m_private->uninit();
    m_bInit = false;
}

void core_filter_base::setFlag(int index)
{
    core_util::setBit(m_flag, index);
}

bool core_filter_base::testFlag(int index)
{
    return core_util::isSetBit(m_flag, index);
}

AVFrame* core_filter_base::mix(AVFrame* in)
{
    if(m_flag)
    {
        update();
        m_flag = 0;
    }
    return m_private->mix(in);
}
