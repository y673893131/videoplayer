#include "core_decoder_hardware.h"

core_decoder_hardware::core_decoder_hardware()
    : m_bFirstSeek(false)
    , m_devType(AV_HWDEVICE_TYPE_NONE)
    , m_buffer(nullptr)
    , m_hwframe(nullptr)
    , m_pixFormat(AV_PIX_FMT_NONE)
{
}

core_decoder_hardware::~core_decoder_hardware()
{
    uninit();
}

bool core_decoder_hardware::initCuda(AVFormatContext *formatCtx, int index)
{
    uninit();
    format = formatCtx;
    nStreamIndex = index;
    if(nStreamIndex < 0)
        return false;
    stream = format->streams[nStreamIndex];
    pCodec = avcodec_find_decoder(stream->codecpar->codec_id);

    for(int i = 0;;++i)
    {
        auto config = avcodec_get_hw_config(pCodec, i);
        if(!config)
        {
            Log(Log_Err, "avcodec_get_hw_config failed!");
            return false;
        }

        if(config->device_type == AV_HWDEVICE_TYPE_CUDA)
        {
            m_pixFormat = config->pix_fmt;
            m_devType = config->device_type;
            break;
        }
    }

    pCodecContext = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecContext, stream->codecpar);

    pCodecContext->opaque = this;
    pCodecContext->get_format = core_decoder_hardware::getFormatCUDA;

    auto ret = av_hwdevice_ctx_create(&m_buffer, m_devType, nullptr, nullptr, 0);
    if(ret < 0)
    {
        avcodec_free_context(&pCodecContext);
        pCodecContext = nullptr;
        char buf[1024] = {};
        av_strerror(ret, buf, 1024);
        Log(Log_Err, "av_hwdevice_ctx_create failed![%s]", buf);
        return false;
    }

    pCodecContext->hw_device_ctx = av_buffer_ref(m_buffer);
    ret = avcodec_open2(pCodecContext, pCodec, nullptr);
    if(ret < 0)
    {
        Log(Log_Err, "avcodec_open2 failed(%d)!", ret);
        return false;
    }

    frame = av_frame_alloc();
    m_hwframe = av_frame_alloc();
    return true;
}

void core_decoder_hardware::uninit()
{
    core_decoder::uninit();
    if(m_hwframe)
    {
        av_frame_free(&m_hwframe);
        m_hwframe = nullptr;
    }

    if(m_buffer)
    {
        av_buffer_unref(&m_buffer);
        m_buffer = nullptr;
    }

    m_devType = AV_HWDEVICE_TYPE_NONE;
    m_pixFormat = AV_PIX_FMT_NONE;
}

bool core_decoder_hardware::decode(AVPacket *pk)
{
    if(!m_hwframe)
        return false;
    auto ret = avcodec_send_packet(pCodecContext, pk);
    if(ret != 0 && ret != AVERROR(EAGAIN))
    {
        Log(Log_Warning, "send packet failed[%d]!", ret);
        return false;
    }

    ret = avcodec_receive_frame(pCodecContext, m_hwframe);
    if(!ret)
    {
        ret = av_hwframe_transfer_data(frame, m_hwframe, 0);
        if(ret < 0)
        {
            return false;
        }

        av_frame_copy_props(frame, m_hwframe);
        calcClock();
        return true;
    }

    return false;
}

bool core_decoder_hardware::checkSeekPkt(AVPacket *pk)
{
    if(!m_bFirstSeek && !(pk->flags & AV_PKT_FLAG_KEY))
    {
        return true;
    }

    m_bFirstSeek = true;

    if(!core_decoder_hardware::decode(pk))
        return true;

    if(frame->pict_type != AV_PICTURE_TYPE_I)
        return true;

    m_bFirstSeek = false;

    return false;
}

void core_decoder_hardware::calcClock()
{
    double pts_video = 0.0;
//            if(pk.dts == AV_NOPTS_VALUE && frame->opaque && *(uint64_t*)frame->opaque != AV_NOPTS_VALUE)
//                pts_video = *reinterpret_cast<double*>(frame->opaque);
//            else if(pk.dts != AV_NOPTS_VALUE)
//                pts_video = static_cast<double>(pk.dts);
//            else
//                pts_video = 0;

    if(frame->pts != AV_NOPTS_VALUE)
        pts_video = frame->pts;
    else if(frame->pkt_dts != AV_NOPTS_VALUE)
        pts_video = frame->pkt_dts;
    else
    {
        auto opa = *reinterpret_cast<double*>(frame->opaque);
        if(opa != AV_NOPTS_VALUE)
            pts_video = opa;
        else
            pts_video = 0;
    }

    pts_video *= av_q2d(stream->time_base);
    _clock = pts_video;
}

AVPixelFormat core_decoder_hardware::getFormatCUDA(AVCodecContext *ctx, const AVPixelFormat *srcFormat)
{
    auto pThis = reinterpret_cast<core_decoder_hardware*>(ctx->opaque);
    for(auto p = srcFormat; *p != -1; ++p)
    {
        if(*p == pThis->m_pixFormat)
            return *p;
    }

    return AV_PIX_FMT_NONE;
}

AVPixelFormat core_decoder_hardware::getFormatQSV(AVCodecContext *ctx, const AVPixelFormat *srcFormat)
{
    auto pThis = reinterpret_cast<core_decoder_hardware*>(ctx->opaque);
    for(auto p = srcFormat; *p != -1; ++p)
    {
        if(*p == pThis->m_pixFormat && *p == AV_PIX_FMT_QSV)
        {
            if(pThis->initQSV(ctx) < 0)
                return AV_PIX_FMT_NONE;
            return *p;
        }
    }

    return AV_PIX_FMT_NONE;
}

int core_decoder_hardware::initQSV(AVCodecContext *ctx)
{
    auto pThis = reinterpret_cast<core_decoder_hardware*>(ctx->opaque);

    ctx->hw_frames_ctx = av_hwframe_ctx_alloc(pThis->m_buffer);
    if (!ctx->hw_frames_ctx)
        return -1;
    auto frames_ctx = reinterpret_cast<AVHWFramesContext*>(ctx->hw_frames_ctx->data);
    auto hw_ctx = reinterpret_cast<AVQSVFramesContext*>(frames_ctx->hwctx);

    frames_ctx->format = AV_PIX_FMT_QSV;
    frames_ctx->sw_format = ctx->sw_pix_fmt;
    frames_ctx->width = FFALIGN(ctx->coded_width, 32);
    frames_ctx->height = FFALIGN(ctx->coded_height, 32);
    frames_ctx->initial_pool_size = /*16*/32;

    hw_ctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;
    return av_hwframe_ctx_init(ctx->hw_frames_ctx);
}

bool core_decoder_hardware::initQsv(AVFormatContext *formatCtx, int index)
{
    uninit();
    format = formatCtx;
    nStreamIndex = index;
    if(nStreamIndex < 0)
        return false;
    stream = format->streams[nStreamIndex];

    int ret = av_hwdevice_ctx_create(&m_buffer, AV_HWDEVICE_TYPE_QSV, "auto", nullptr, 0);
    if (ret < 0)
    {
        return false;
    }

    pCodec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!pCodec)
    {
        return false;
    }

    pCodec = avcodec_find_decoder_by_name((std::string(pCodec->name) + "_qsv").c_str());
    if (!pCodec)
    {
        return false;
    }

//    auto format = stream->codecpar->format;
//    auto formats = pCodec->pix_fmts;
//    Log(Log_Info, "%s[%d_%d]", pCodec->name, stream->codecpar->format, pCodec->pix_fmts[0]);

    for (int i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(pCodec, i);
        if (!config)
        {
            Log(Log_Err, "Decoder %s does not support device type %s.", pCodec->name, av_hwdevice_get_type_name(AV_HWDEVICE_TYPE_QSV));
            return false;
        }

        if (config->device_type == AV_HWDEVICE_TYPE_QSV)
        {
            m_pixFormat = config->pix_fmt;
            m_devType = AV_HWDEVICE_TYPE_QSV;
            break;
        }
    }

    pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext)
        return false;
    if (avcodec_parameters_to_context(pCodecContext, stream->codecpar) < 0)
        return false;
    pCodecContext->opaque = this;
    pCodecContext->get_format = core_decoder_hardware::getFormatQSV;
    pCodecContext->pix_fmt = AV_PIX_FMT_NV12;

    ret = avcodec_open2(pCodecContext, nullptr, nullptr);
    if (ret < 0)
        return false;

    frame = av_frame_alloc();
    m_hwframe = av_frame_alloc();
    return true;
}
