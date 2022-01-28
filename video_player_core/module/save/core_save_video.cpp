#include "core_save_video.h"
#include "../util/core_util.h"
#include "../media/core_media.h"
#include "../common.h"
#include "Log/Log.h"

core_save_video::core_save_video()
    : core_save_base()
{
}

core_save_video::~core_save_video()
{
    uninit();
}

bool core_save_video::init(core_media* media, int nIndex)
{
    uninit();

    if(!core_save_base::init(media, nIndex))
    {
        uninit();
        return false;
    }

    return true;
}

std::string core_save_video::outoutFile()
{
    return core_util::toDateTime(time(nullptr)) + ".h264";
}

void core_save_video::uninit()
{
    core_save_base::uninit();
}

void core_save_video::save(AVPacket* pk)
{
    if(!m_format)
        return;
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
