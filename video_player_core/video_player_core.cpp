#include "video_player_core.h"
#include "Log/Log.h"
#include "video_define.h"
#include "video_thread.h"

#include <memory>

video_player_core::video_player_core()
{
    m_info = new _video_info_();
    m_info->init();
    setBit(m_info->_flag, flag_bit_Stop);
}

video_player_core::~video_player_core()
{
    SAFE_RELEASE_PTR(&m_info);
    avformat_network_deinit();
}

int video_player_core::_init()
{
#ifndef AVFORMAT_STATIC_REGISTER
    av_register_all();
#endif
    Log(Log_Info, "avformat_network_init()=%d", avformat_network_init());
    return 0;
}

int video_player_core::_uninit()
{
    Log(Log_Info, "%s", __FUNCTION__);

//    if(_getState() != state_running)
//        _stop();

    return 0;
}

int video_player_core::_setCallBack(video_interface * cb)
{
    Log(Log_Info, "%s", __FUNCTION__);
    m_info->_cb = cb;
    return 0;
}

int video_player_core::_setSrc(const std::string &src)
{
    Log(Log_Opt, "%s=%s", __FUNCTION__, src.c_str());
    m_info->src = src;
    return 0;
}

int video_player_core::_play()
{
    Log(Log_Opt, "%s call.", __FUNCTION__);
    setBit(m_info->_flag, flag_bit_Stop, false);
    setBit(m_info->_flag, flag_bit_pause, false);
    setBit(m_info->_flag, flag_bit_need_pause, false);
    video_thread::start(*m_info);
    return 0;
}

int video_player_core::_pause(int index)
{
    Log(Log_Opt, "%s call.", __FUNCTION__);
    auto pIndex = video_thread::index(index);
    if(pIndex)
        pIndex->setPause();
    return 0;
}

int video_player_core::_continue(int index)
{
    Log(Log_Opt, "%s call.", __FUNCTION__);
    auto pIndex = video_thread::index(index);
    if(pIndex)
        pIndex->continuePlay();
    return 0;
}

int video_player_core::_stop(int index)
{
    Log(Log_Opt, "%s call.", __FUNCTION__);
    auto pIndex = video_thread::index(index);
    if(pIndex)
        pIndex->setStop();
    return 0;
}

int video_player_core::_seek(int index, int64_t ms)
{
    Log(Log_Opt, "%s=%lld", __FUNCTION__, ms);
    auto pIndex = video_thread::index(index);
    if(pIndex)
        pIndex->seekPos(ms);
    return 0;
}

int video_player_core::_setVol(int index, int nVol)
{
    Log(Log_Opt, "%s=%d", __FUNCTION__, nVol);
    if(m_info && m_info->audio)
    {
        nVol = static_cast<int>(nVol / 100.0 * SDL_MIX_MAXVOLUME);
        m_info->audio->sdl->nVol = nVol;
    }
    auto pIndex = video_thread::index(index);
    if(pIndex)
        pIndex->setVol(nVol);
    return 0;
}

int video_player_core::_setMute(int index, bool bMute)
{
    Log(Log_Opt, "%s=%d", __FUNCTION__, bMute);
    setBit(m_info->_flag, flag_bit_mute, bMute);
    auto pIndex = video_thread::index(index);
    if(pIndex)
        pIndex->setMute(bMute);
    return 0;
}

int video_player_core::_setsize(int w, int h)
{

    Log(Log_Opt, "%s(%d,%d)", __FUNCTION__, w, h);
    if(m_info->yuv)
        m_info->yuv->setSize(w, h);
    return 0;
}

int video_player_core::_state(int index)
{
    Log(Log_Opt, "%s[%d]", __FUNCTION__, index);
    auto pIndex = video_thread::index(index);
    if(pIndex)
    {
        Log(Log_Opt, "%s=%d", __FUNCTION__, pIndex->state());
        return pIndex->state();
    }

    return state_uninit;
}

video_player_core::enum_state video_player_core::_getState()
{
    if(isSetBit(m_info->_flag, flag_bit_pause) ||
       isSetBit(m_info->_flag, flag_bit_need_pause))
        return state_paused;
    if(isSetBit(m_info->_flag, flag_bit_Stop))
        return state_stopped;
    return state_running;
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
