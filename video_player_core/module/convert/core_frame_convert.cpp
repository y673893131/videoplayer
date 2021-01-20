#include "core_frame_convert.h"

core_frame_convert::core_frame_convert()
    :frame(nullptr)
    ,buffer(nullptr)
    ,swsCov(nullptr)
    ,srcCodec(nullptr)
    ,size(0)
    ,w(0)
    ,h(0)
    ,srcWidth(0)
    ,srcHeight(0)
    ,picflag(AV_PIX_FMT_YUV420P)
{
}

core_frame_convert::core_frame_convert(AVCodecContext *codec, AVPixelFormat format)
    :frame(nullptr)
    ,buffer(nullptr)
    ,swsCov(nullptr)
    ,srcCodec(nullptr)
    ,size(0)
    ,w(0)
    ,h(0)
    ,srcWidth(0)
    ,srcHeight(0)
    ,picflag(format)
{
    setsrcCodec(codec);
}

core_frame_convert::~core_frame_convert()
{
    uninit();
}

void core_frame_convert::setSize(int w, int h)
{
    init(w, h);
}

int core_frame_convert::width()
{
    return w;
}

int core_frame_convert::height()
{
    return h;
}
//#ifdef FRAME_RGB
//#define SELECT_PIX_FMT AV_PIX_FMT_RGBA
//#else
//#define SELECT_PIX_FMT AV_PIX_FMT_YUV420P
//#endif
void core_frame_convert::setsrcCodec(AVCodecContext* codec)
{
    srcCodec = codec;
    init();
}

void core_frame_convert::init(int width /*= 0*/, int height /*= 0*/)
{
    LOCK(mutex)
    Log(Log_Info, "buffer0:%p, w=%d, h=%d", buffer, w, h);
    uninit();
    if(width) w = width;
    if(height) h = height;

    if(!srcCodec)
        return;

    initFrame();
}

void core_frame_convert::uninit()
{
    if(frame)
    {
        av_frame_free(&frame);
        frame = nullptr;
    }

    if(buffer)
    {
        av_free(buffer);
        buffer = nullptr;
    }

    if(swsCov)
    {
        sws_freeContext(swsCov);
        swsCov = nullptr;
    }
}

void core_frame_convert::initFrame()
{
    uninit();
    if(!w) w = srcCodec->width;
    if(!h) h = srcCodec->height;
    // 16pix align
    w = (w >> 4) << 4;
    h = (h >> 4) << 4;
    srcWidth = srcCodec->width;
    srcHeight = srcCodec->height;
    frame = av_frame_alloc();
    swsCov = sws_getContext(srcCodec->width, srcCodec->height, srcCodec->pix_fmt
                   , w, h, picflag
                   , SWS_BICUBIC, nullptr, nullptr, nullptr);
    size = avpicture_get_size(picflag, w, h);
    buffer = reinterpret_cast<uint8_t*>(av_malloc(size * sizeof(uint8_t)));
    avpicture_fill(reinterpret_cast<AVPicture*>(frame), buffer, picflag, w, h);
    Log(Log_Info, "srcWidth=%d, srcHeight=%d size=%d, w=%d, h=%d", srcWidth, srcHeight, size, w, h);
}

void core_frame_convert::scalePreview(AVFrame *src, void *cb)
{
    if(srcWidth != srcCodec->width || srcHeight != srcCodec->height)
        initFrame();
    sws_scale(swsCov,reinterpret_cast<const uint8_t *const *>(src->data)
              ,src->linesize, 0, srcCodec->height, frame->data
              ,frame->linesize);
//        Log(Log_Info, "src_line_size:[%d-%d-%d], width: %d, height: %d dst_line_size:[%d-%d-%d]", src->linesize[0], src->linesize[1], src->linesize[2]
//                , w, h, frame->linesize[0], frame->linesize[1], frame->linesize[2]);
    if(cb)
    {
        reinterpret_cast<video_interface*>(cb)->previewDisplayCall(buffer, w, h);
    }
}

void core_frame_convert::scale(AVFrame* src, void* cb)
{
    if(srcWidth != srcCodec->width || srcHeight != srcCodec->height)
        initFrame();
    sws_scale(swsCov,reinterpret_cast<const uint8_t *const *>(src->data)
              ,src->linesize, 0, srcCodec->height, frame->data
              ,frame->linesize);
//        Log(Log_Info, "src_line_size:[%d-%d-%d], width: %d, height: %d dst_line_size:[%d-%d-%d]", src->linesize[0], src->linesize[1], src->linesize[2]
//                , w, h, frame->linesize[0], frame->linesize[1], frame->linesize[2]);
    if(cb)
        reinterpret_cast<video_interface*>(cb)->displayCall(buffer, w, h);
}
