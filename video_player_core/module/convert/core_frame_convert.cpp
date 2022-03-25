#include "core_frame_convert.h"
#include "video_player_core.h"

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
#ifdef FRAME_RGB
    #define SELECT_PIX_FMT AV_PIX_FMT_RGBA
    ,picflag(AV_PIX_FMT_RGBA)
#else
#define SELECT_PIX_FMT AV_PIX_FMT_YUV420P
    ,picflag(AV_PIX_FMT_YUV420P)
#endif

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

void core_frame_convert::setsrcCodec(AVCodecContext* codec)
{
    srcCodec = codec;
    init();
}

AVCodecContext* core_frame_convert::src()
{
    return srcCodec;
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
//    w = FFALIGN(w, 64);
//    h = FFALIGN(h, 16);
    srcWidth = srcCodec->width;
    srcHeight = srcCodec->height;
    frame = av_frame_alloc();

    initBuffer();
}

void core_frame_convert::scalePreview(AVFrame *src, video_interface *cb)
{
    scaleFrame(src);
    if(cb)
    {
        cb->previewDisplayCall(buffer, srcWidth, srcHeight);
    }
}

void core_frame_convert::scale(AVFrame* src, video_interface* cb)
{
    scaleFrame(src);
    if(cb)
    {
        cb->displayCall(buffer, srcWidth, srcHeight);
    }
}

void core_frame_convert::scaleFrame(AVFrame* src)
{
    if(srcWidth != srcCodec->width || srcHeight != srcCodec->height)
        initFrame();

//    LogB(Log_Debug, "src_line_size:[%d-%d-%d], format=%d w=%d, h=%d dst_line_size:[%d-%d-%d] format=%d w=%d h=%d", src->linesize[0], src->linesize[1], src->linesize[2]
//            , src->format, src->width, src->height, frame->linesize[0], frame->linesize[1], frame->linesize[2]
//            , picflag, w, h);

    if(w != src->linesize[0])
    {
        if(buffer)
        {
            av_free(buffer);
        }

        w = src->linesize[0];
        initBuffer();
    }

    auto flags = (src->width == w && src->height == h) ? SWS_POINT : SWS_FAST_BILINEAR;
    swsCov = sws_getCachedContext(swsCov
                , src->width, src->height, static_cast<AVPixelFormat>(src->format)
                , srcWidth, srcHeight, picflag
                , flags
                , nullptr, nullptr, nullptr
                );
    sws_scale(
        swsCov,
        reinterpret_cast<const uint8_t *const *>(src->data),
        src->linesize,
        0,
        src->height,
        frame->data,
        frame->linesize);

    int ret = av_image_copy_to_buffer(
        buffer,
        static_cast<int>(sizeReal),
        frame->data,
        frame->linesize, picflag,
        srcWidth, srcHeight, 1);
    if(ret < 0)
    {
        char error[1024] ={};
        av_strerror(ret, error, 1024);
        LogB(Log_Err, error);
    }
}

void core_frame_convert::initBuffer()
{
    sizeReal = static_cast<unsigned int>(av_image_get_buffer_size(picflag, srcWidth, srcHeight, 1));
    size = static_cast<unsigned int>(av_image_get_buffer_size(picflag, w, h, 1));
    buffer = reinterpret_cast<uint8_t*>(av_malloc(size * sizeof(uint8_t)));
    av_image_fill_arrays(frame->data, frame->linesize, buffer, picflag, w, h, 1);
    Log(Log_Info, "pix_fmt=%d srcWidth=%d, srcHeight=%d size=%d, w=%d, h=%d", srcCodec->pix_fmt, srcWidth, srcHeight, size, w, h);
}
