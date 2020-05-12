#include "video_player_core.h"
#include "Log/Log.h"
#include "video_define.h"
#include "video_thread.h"
#include <memory>

video_player_core::video_player_core()
{
    m_info = new _video_info_();
    setBit(m_info->_flag, flag_bit_Stop);
}

video_player_core::~video_player_core()
{
    if(m_info){
        delete m_info;
        m_info = nullptr;
    }
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
    if(_getState() != state_running)
        _stop();

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
    video_thread::start(m_info);
    return 0;
}

int video_player_core::_pause()
{
    Log(Log_Opt, "%s call.", __FUNCTION__);
    if(!isSetBit(m_info->_flag, flag_bit_pause)){
        setBit(m_info->_flag, flag_bit_need_pause);
    }
    return 0;
}

int video_player_core::_continue()
{
    Log(Log_Opt, "%s call.", __FUNCTION__);
    if(isSetBit(m_info->_flag, flag_bit_pause)){
        setBit(m_info->_flag, flag_bit_need_pause, false);
        setBit(m_info->_flag, flag_bit_pause, false);
    }
    return 0;
}

int video_player_core::_stop()
{
    Log(Log_Opt, "%s call.", __FUNCTION__);
    setBit(m_info->_flag, flag_bit_Stop);
    return 0;
}

int video_player_core::_seek(_int64 ms)
{
    Log(Log_Opt, "%s=%I64d", __FUNCTION__, ms);
    if(!isSetBit(m_info->_flag, flag_bit_seek))
    {
        m_info->_seek_pos = ms;
        setBit(m_info->_flag, flag_bit_seek);
    }
    return 0;
}

int video_player_core::_setsize(int w, int h)
{
    // 16pix align
    w = (w >> 4) << 4;
    h = (h >> 4) << 4;
    Log(Log_Opt, "%s(%d,%d)", __FUNCTION__, w, h);
    m_info->yuv.setSize(w, h);
    return 0;
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

    AVFrame* srcFrame;  //编码前数据，如yuv420，rgb24等
    AVFrame* dstFrame;  //编码前数据，如yuv420，rgb24等

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

    //获取存储媒体数据空间
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
