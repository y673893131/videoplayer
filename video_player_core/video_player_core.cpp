#include "video_player_core.h"
#include "Log/Log.h"
//#include "video_define.h"
#include "module/media/core_media.h"
#include "module/thread/core_thread_demux.h"

#include <memory>

video_player_core::video_player_core(const std::string& logDir)
{
    InitLogInstance(logDir.c_str(), "log_core_");
    m_media = new core_media();
    m_media->init();
}

video_player_core::~video_player_core()
{
    SAFE_RELEASE_PTR(&m_media);
    avformat_network_deinit();
}

int video_player_core::_init()
{
#ifndef AVFORMAT_STATIC_REGISTER
#if defined (USE_DESKTOP) || defined (USE_CAMERA)
    avdevice_register_all();
#endif
    avcodec_register_all();
    av_register_all();
#endif
    Log(Log_Info, "avformat_network_init()=%d", avformat_network_init());
    return 0;
}

int video_player_core::_uninit()
{
    Log(Log_Info, ".");

//    if(_getState() != state_running)
//        _stop();

    return 0;
}

int video_player_core::_setCallBack(video_interface * cb)
{
    Log(Log_Info, ".");
    m_media->setCallback(cb);
    return 0;
}

int video_player_core::_setSrc(const std::string &src)
{
    m_media->setSrc(src);
    return 0;
}

int video_player_core::_play()
{
    Log(Log_Opt, "call.");
    core_thread_demux::start(*m_media);
    return 0;
}

int video_player_core::_pause(int index)
{
//    Log(Log_Opt, "call.");
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
        pIndex->setPause();
    return 0;
}

int video_player_core::_continue(int index)
{
//    Log(Log_Opt, "call.");
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
        pIndex->continuePlay();
    return 0;
}

int video_player_core::_stop(int index)
{
    Log(Log_Opt, "call.");
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
        pIndex->setStop();
    return 0;
}

int video_player_core::_seek(int index, int64_t ms)
{
//    Log(Log_Opt, "%lld", ms);
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
        pIndex->seekPos(ms);
    return 0;
}

bool video_player_core::_seekJump(int index, int64_t ms)
{
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
        return pIndex->seekJump(ms);
    return false;
}

int video_player_core::_get_seek_img(int index, int64_t ms)
{
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
        pIndex->getSeekImg(ms);
    return 0;
}

int video_player_core::_setVol(int index, int nVol)
{
//    Log(Log_Opt, "%d", nVol);
    if(m_media)
        m_media->setVol(nVol);
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
        pIndex->setVol(nVol);
    return 0;
}

int video_player_core::_setMute(int index, bool bMute)
{
    Log(Log_Opt, "%d", bMute);
    if(m_media)
        m_media->mute(bMute);

    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
        pIndex->setMute(bMute);
    return 0;
}

int video_player_core::_setAudioChannel(int index, audio_channel_type type)
{
    Log(Log_Opt, "%d", type);
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
        pIndex->setAudioChannel(type);
    return 0;
}

int video_player_core::_setStreamChannel(int index, int channel, int sel)
{
    Log(Log_Opt, " select (%d,%d)", channel, sel);
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
    {
        pIndex->setChannel(channel, sel);
    }
    return 0;
}

int video_player_core::_setDecodeType(int index, int type)
{
    Log(Log_Opt, "(%d)", type);
    if(m_media)
        m_media->setDecode(type);

    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
    {
        pIndex->setDecode(type);
    }

    return 0;
}

int video_player_core::_setSpeedType(int index, int type)
{
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
        pIndex->setSpeed(type);
    if(m_media)
        m_media->setSpeed(type);
    return 0;
}

int video_player_core::_state(int index)
{
//    Log(Log_Opt, "[%d]", index);
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
    {
//        Log(Log_Opt, "%d", pIndex->state());
        return pIndex->state();
    }

    return state_uninit;
}

video_player_core::enum_state video_player_core::_getState(int index)
{
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
    {
        Log(Log_Opt, "%d", pIndex->state1());
        return pIndex->state1();
    }

    return state_stopped;
}

void *video_player_core::_frame(int index)
{
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
    {
        return pIndex->frame();
    }

    return  nullptr;
}

bool pixel_format_convert(unsigned char *pdata_src, int src_width, int src_height, AVPixelFormat src_pixfmt,
    unsigned char *pdata_dst, int dst_size, AVPixelFormat dst_pixfmt)
{
    const int src_w = src_width, src_h = src_height;
    const int dst_w = src_width, dst_h = src_height;

    AVFrame* srcFrame;
    AVFrame* dstFrame;

    struct SwsContext *img_convert_ctx;
    img_convert_ctx = sws_alloc_context();

    //Set Value
    av_opt_set_int(img_convert_ctx, "sws_flags", SWS_BICUBIC | SWS_PRINT_INFO, 0);
    av_opt_set_int(img_convert_ctx, "srcw", src_w, 0);
    av_opt_set_int(img_convert_ctx, "srch", src_h, 0);
    av_opt_set_int(img_convert_ctx, "src_format", src_pixfmt, 0);
    //'0' for MPEG (Y:0-235);'1' for JPEG (Y:0-255)
    av_opt_set_int(img_convert_ctx, "src_range", 1, 0);
    av_opt_set_int(img_convert_ctx, "dstw", dst_w, 0);
    av_opt_set_int(img_convert_ctx, "dsth", dst_h, 0);
    av_opt_set_int(img_convert_ctx, "dst_format", dst_pixfmt, 0);
    av_opt_set_int(img_convert_ctx, "dst_range", 1, 0);
    sws_init_context(img_convert_ctx, NULL, NULL);

    srcFrame = av_frame_alloc();
    if (NULL != srcFrame)
    {
        srcFrame->width = src_w;
        srcFrame->height = src_h;
        srcFrame->format = AV_PIX_FMT_YUV420P;
    }

    if (av_frame_get_buffer(srcFrame, 1) < 0)
    {
        printf("get media buff failure.\n");
        return false;
    }

    av_image_fill_arrays(srcFrame->data, srcFrame->linesize, pdata_src, AV_PIX_FMT_YUV420P, srcFrame->width, srcFrame->height, 1);

    dstFrame = av_frame_alloc();
    if (NULL != dstFrame)
    {
        dstFrame->width = src_w;
        dstFrame->height = src_h;
        dstFrame->format = AV_PIX_FMT_RGB24;
    }

    if (av_frame_get_buffer(dstFrame, 1) < 0)
    {
        printf("get media buff failure.\n");
        return false;
    }

    //do transform
    sws_scale(img_convert_ctx, srcFrame->data, srcFrame->linesize, 0, dstFrame->height, dstFrame->data, dstFrame->linesize);

    av_image_copy_to_buffer(pdata_dst, dst_size, dstFrame->data, dstFrame->linesize, AV_PIX_FMT_RGB24, dstFrame->width, dstFrame->height, 1);

    av_frame_free(&srcFrame);
    av_frame_free(&dstFrame);

    sws_freeContext(img_convert_ctx);

    return true;
}

bool video_player_core::_cov(void *indata, void* outdata, int w, int h, int outsize)
{
    return pixel_format_convert((unsigned char*)indata, w, h, AV_PIX_FMT_YUV420P,
                                (unsigned char*)outdata, outsize, AV_PIX_FMT_RGB24);
}

int video_player_core::_setCapture(int index, bool bCap)
{
    Log(Log_Opt, "%d", bCap);
    auto pIndex = core_thread_demux::index(index);
    if(pIndex)
    {
        pIndex->setCapture(bCap);
    }

    if(m_media)
        m_media->setCapture(bCap);

    return 0;
}
