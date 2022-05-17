#include "core_convert_video.h"
#include "core_convert_p.h"
#include "../lock/core_graud_lock.h"

class core_convert_videoPrivate : public core_convertPrivate
{
    VP_DECLARE_PUBLIC(core_convert_video)
    inline core_convert_videoPrivate(core_convert_video* parent)
        : core_convertPrivate(parent)
    {
        init();
    }

    void init()
    {
        m_buffer    = nullptr;
        m_swsCov    = nullptr;
        m_codecCtx  = nullptr;
        m_size      = 0;
        m_sizeReal  = 0;
        m_width     = 0;
        m_height    = 0;
        m_srcWidth  = 0;
        m_srcHeight = 0;
#ifdef FRAME_RGB
#define SELECT_PIX_FMT AV_PIX_FMT_RGBA
        m_pixelFormat = AV_PIX_FMT_RGBA;
#else
#define SELECT_PIX_FMT AV_PIX_FMT_YUV420P
        m_pixelFormat = AV_PIX_FMT_YUV420P;
#endif
    }

    void initConvert();
    void unInitConvert();

    void initFrame() override;
    void initBuffer();
    void unInitBuffer();

    void checkFrameSize(AVFrame *);
    bool initResample() override;
    bool convert(AVFrame *) override;
    core_dev *dev() override;
    void reset() override;

    SwsContext      *m_swsCov;
    core_graud_lock m_mutex;
    AVPixelFormat   m_pixelFormat;
    unsigned int    m_sizeReal;
    int             m_width;
    int             m_height;
    int             m_srcWidth;
    int             m_srcHeight;
};

void core_convert_videoPrivate::initConvert()
{
    unInitConvert();
    if(!m_codecCtx)
        return;
    initFrame();
}

void core_convert_videoPrivate::unInitConvert()
{
    unInitFrame();
    unInitBuffer();

    if(m_swsCov)
    {
        sws_freeContext(m_swsCov);
        m_swsCov = nullptr;
    }
}

void core_convert_videoPrivate::initFrame()
{
    unInitConvert();
    core_convertPrivate::initFrame();
    m_srcWidth = m_codecCtx->width;
    m_srcHeight = m_codecCtx->height;
    if(!m_width) m_width = m_codecCtx->width;
    if(!m_height) m_height = m_codecCtx->height;
    initBuffer();
}

void core_convert_videoPrivate::initBuffer()
{
    m_sizeReal = static_cast<unsigned int>(av_image_get_buffer_size(m_pixelFormat, m_codecCtx->width, m_codecCtx->height, 1));
    m_size = static_cast<unsigned int>(av_image_get_buffer_size(m_pixelFormat, m_width, m_height, 1));
    m_buffer = reinterpret_cast<uint8_t*>(av_malloc(m_size * sizeof(uint8_t)));
    av_image_fill_arrays(m_frame->data, m_frame->linesize, m_buffer, m_pixelFormat, m_width, m_height, 1);
    Log(Log_Info, "pix_fmt=%d srcWidth=%d, srcHeight=%d size=%d, w=%d, h=%d", m_codecCtx->pix_fmt, m_codecCtx->width, m_codecCtx->height, m_size, m_width, m_height);
}

void core_convert_videoPrivate::unInitBuffer()
{
    if(m_buffer)
    {
        av_free(m_buffer);
        m_buffer = nullptr;
    }
}

void core_convert_videoPrivate::checkFrameSize(AVFrame* src)
{
    if(m_srcWidth != m_codecCtx->width || m_srcHeight != m_codecCtx->height)
        initFrame();

    if(m_width != src->linesize[0])
    {
        if(m_buffer)
        {
            av_free(m_buffer);
        }

        m_width = src->linesize[0];
        initBuffer();
    }
}

bool core_convert_videoPrivate::initResample()
{
    return true;
}

bool core_convert_videoPrivate::convert(AVFrame *src)
{
    checkFrameSize(src);
    auto flags = (src->width == m_width && src->height == m_height) ? SWS_POINT : SWS_FAST_BILINEAR;
    m_swsCov = sws_getCachedContext(m_swsCov
                , src->width, src->height, static_cast<AVPixelFormat>(src->format)
                , m_srcWidth, m_srcHeight, m_pixelFormat
                , flags
                , nullptr, nullptr, nullptr
                );
    sws_scale(
        m_swsCov,
        reinterpret_cast<const uint8_t *const *>(src->data),
        src->linesize,
        0,
        src->height,
        m_frame->data,
        m_frame->linesize);

    int ret = av_image_copy_to_buffer(
        m_buffer,
        static_cast<int>(m_sizeReal),
        m_frame->data,
        m_frame->linesize, m_pixelFormat,
        m_srcWidth, m_srcHeight, 1);
    if(ret < 0)
    {
        char error[1024] ={};
        av_strerror(ret, error, 1024);
        LogB(Log_Err, error);
        return false;
    }

    return true;
}

core_dev *core_convert_videoPrivate::dev()
{
    return nullptr;
}

void core_convert_videoPrivate::reset()
{
    return;
}

core_convert_video::core_convert_video()
    : core_convert(new core_convert_videoPrivate(this))
{
    VP_D(core_convert_video);
    d->initConvert();
}
