#include "core_filter.h"
#include "../util/core_util.h"
#include "../media/core_media.h"
#include "../common.h"
#include "Log/Log.h"
#include "core_filter_audio.h"
#include "core_filter_video.h"

core_filter::core_filter()
    : m_filter(nullptr)
    , m_filterTmp(nullptr)
    , m_pStream(nullptr)
    , m_pCodecContext(nullptr)
{
}

core_filter::~core_filter()
{

}

template<class T>
bool core_filter::init(AVStream* pStream, AVCodecContext* pCodecContext)
{
    uninit();
    avfilter_register_all();

    m_pStream = pStream;
    m_pCodecContext = pCodecContext;

    INIT_NEW(&m_filter, T)

    return true;
}

void core_filter::uninit()
{
    SAFE_RELEASE_PTR(&m_filter)
    SAFE_RELEASE_PTR(&m_filterTmp)
}

void core_filter::setValue(core_filter_type type, int)
{
    switch (type) {
    case filter_volume:
        break;
    }
}

AVFrame* core_filter::mix(AVFrame* in)
{
    return m_filter->mix(in);
}
