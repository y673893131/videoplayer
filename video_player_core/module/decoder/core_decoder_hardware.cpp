#include "core_decoder_hardware.h"
#include "video_player_core.h"

core_decoder_hardware::core_decoder_hardware()
    : m_bFirstSeek(false)
    , m_decodeType(AV_HWDEVICE_TYPE_NONE)//default is software
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

std::map<int, std::string> core_decoder_hardware::getSupportDevices()
{
    std::map<int, std::string> devs;

    AVBufferRef* buffer;
    auto tmp = pCodec;
    for(int i = 0;;++i)
    {
        auto config = avcodec_get_hw_config(tmp, i);
        if(!config)
        {
            if(!devs.empty())
            {
                Log(Log_Err, "avcodec_get_hw_config failed!");
            }
            break;
        }

        if(config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)
        {
            auto ret = av_hwdevice_ctx_create(&buffer, config->device_type, nullptr, nullptr, 0);
            if(ret >= 0)
            {
                av_buffer_unref(&buffer);
                auto name = av_get_pix_fmt_name(config->pix_fmt);
                devs.insert(std::make_pair(config->device_type, name));
            }
        }
    }

    //qsv diff
    int ret = av_hwdevice_ctx_create(&buffer, AV_HWDEVICE_TYPE_QSV, "auto", nullptr, 0);
    if (ret >= 0)
    {
        av_buffer_unref(&buffer);
        devs.insert(std::make_pair(AV_HWDEVICE_TYPE_QSV, "qsv"));
    }

    return std::move(devs);
}

bool core_decoder_hardware::init(AVFormatContext *formatCtx, int index)
{
    if(m_decodeType == AV_HWDEVICE_TYPE_QSV)
        return initQsv(formatCtx, index);
    else
        return initOtherHw(formatCtx, index);
}

bool core_decoder_hardware::initOtherHw(AVFormatContext *formatCtx, int index)
{
    uninit();

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

        if(config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)
        {
            if(config->device_type == m_decodeType)
            {
                m_pixFormat = config->pix_fmt;
                m_devType = config->device_type;
                break;
            }
        }
    }

    pCodecContext = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecContext, stream->codecpar);
    pCodecContext->pkt_timebase = stream->time_base;

    pCodecContext->opaque = this;
    pCodecContext->get_format = core_decoder_hardware::getFormatOtherHw;
    pCodecContext->pix_fmt = AV_PIX_FMT_NV12;
    pCodecContext->thread_count = 5;
    pCodecContext->thread_type = FF_THREAD_FRAME;


    auto ret = av_hwdevice_ctx_create(&m_buffer, m_devType, nullptr, nullptr, 0);
    if(ret < 0)
    {
        avcodec_free_context(&pCodecContext);
        pCodecContext = nullptr;
        char buf[1024] = {};
        av_strerror(ret, buf, 1024);
        Log(Log_Err, "av_hwdevice_ctx_create failed![%d-%s]", ret, buf);
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

bool core_decoder_hardware::decode(AVPacket *pk, bool& bTryAgain)
{
    if(!m_hwframe)
        return false;
    auto ret = avcodec_send_packet(pCodecContext, pk);
    if(ret != 0)
    {
        bTryAgain = (ret == AVERROR(EAGAIN));
        if(!bTryAgain)
        {
            Log(Log_Warning, "send packet failed[%d]!", ret);
            return false;
        }
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
        calcClock(pk);
        return true;
    }

    return false;
}

bool core_decoder_hardware::checkSeekPkt(AVPacket *pk)
{
    if(!m_bFirstSeek && !isIFrame(pk))
    {
        return true;
    }

    m_bFirstSeek = true;
    bool bTryagain = false;
    if(!core_decoder_hardware::decode(pk, bTryagain))
        return true;

    if(frame->pict_type != AV_PICTURE_TYPE_I)
        return true;

    m_bFirstSeek = false;

    return false;
}

bool core_decoder_hardware::isIFrame(AVPacket *pk)
{
    return (pk->flags & AV_PKT_FLAG_KEY);
}

void core_decoder_hardware::checkQSVClock(AVPacket *pk, int64_t &pts)
{
    /*
     *  https://trac.ffmpeg.org/ticket/9095
        h264_qsv bug
        avcodec_flush_buffers later
        av_send_packet->av_recive_frame->frame
        frame->pts is last frame(before seek) pts
        so do this, check AVPacket/AVFrame pts
    */
    if(m_decodeType == AV_HWDEVICE_TYPE_QSV && abs(pts - pk->pts) > 5000)
        pts = pk->pts;
}

AVPixelFormat core_decoder_hardware::getFormatOtherHw(AVCodecContext *ctx, const AVPixelFormat *srcFormat)
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
    frames_ctx->initial_pool_size = 32;

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
        Log(Log_Err, "avcodec_find_decoder[qsv] error");
        return false;
    }

#if PRINT_DECODER
    auto tmp = av_codec_next(nullptr);
    while(tmp)
    {
        auto name = std::string(tmp->name);
        if(((int)name.find("qsv")) > 0)
            LogB(Log_Info, "decoder:[%s]", name.c_str());
        tmp = av_codec_next(tmp);
    }
#endif

    auto sName = std::string(pCodec->name) + "_qsv";
    pCodec = avcodec_find_decoder_by_name(sName.c_str());
    if (!pCodec)
    {
        Log(Log_Err, "avcodec_find_decoder_by_name error, %s", sName.c_str());
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
    pCodecContext->pkt_timebase = stream->time_base;
    pCodecContext->opaque = this;
    pCodecContext->get_format = core_decoder_hardware::getFormatQSV;
    pCodecContext->pix_fmt = AV_PIX_FMT_NV12;

    ret = avcodec_open2(pCodecContext, nullptr, nullptr);
    if(ret < 0)
    {
        Log(Log_Err, "avcodec_open2[qsv] failed(%d)!", ret);
        return false;
    }

    frame = av_frame_alloc();
    m_hwframe = av_frame_alloc();
    return true;
}
