#include "core_save_video.h"
#include "../util/core_util.h"
#include "../media/core_media.h"
#include "../common.h"
#include "Log/Log.h"

core_save_video::core_save_video()
    : core_save_base()
    , m_bsfCtx(nullptr)
{
}

core_save_video::~core_save_video()
{
    uninit();
}

bool core_save_video::init(AVFormatContext* pFormat, int nIndex)
{
    uninit();

    if(!__super::init(pFormat, nIndex))
        return false;


    if(!initBsf())
        return false;

    return true;
}

std::string core_save_video::outoutFile()
{
    return core_util::toTime(time(nullptr)) + ".h264";
}

AVOutputFormat *core_save_video::guess()
{
    return av_guess_format("mp4", NULL, "video/mp4");
}

void core_save_video::uninit()
{
    __super::uninit();

    if(m_bsfCtx)
    {
        av_bsf_free(&m_bsfCtx);
        m_bsfCtx = nullptr;
    }
}

bool core_save_video::initBsf()
{
    return true;
    auto bsf = av_bsf_get_by_name("h264_mp4toannexb");
    if(!bsf)
    {
        return false;
    }

    av_bsf_alloc(bsf, &m_bsfCtx);
    avcodec_parameters_copy(m_bsfCtx->par_in, m_stream->codecpar);
    av_bsf_init(m_bsfCtx);

    return true;
}

void core_save_video::save(AVPacket* pk)
{
    auto _pk = av_packet_clone(pk);
    int ret = 0;
//    int ret = av_bsf_send_packet(m_bsfCtx, _pk);
    if (!ret) {
//        for(;;)
//        {
//            auto ret = av_bsf_receive_packet(m_bsfCtx, _pk);
//            if(ret == EAGAIN)
//            {
//                continue;
//            }
//            else
//            {
//                break;
//            }
//        }

        rescalePacket(_pk);
        auto ret = av_interleaved_write_frame(m_format, _pk);
        if(ret < 0)
        {
            char buf[1024] = {};
            av_strerror(ret, buf, 1024);
            Log(Log_Warning, "[video]av_interleaved_write_frame failed, %s", buf);
        }
    }
    else
    {
        char buf[1024] = {};
        av_strerror(ret, buf, 1024);
        Log(Log_Warning, "[video]av_bsf_send_packet failed, %s", buf);
    }

    av_packet_unref(_pk);
}
