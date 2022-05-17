#include "core_preview.h"
#include "../util/core_util.h"
#include "../decoder/core_decoder_video.h"
#include "../decoder/core_decoder_subtitle.h"
#include "../convert/core_convert_video.h"

class core_previewPrivate : public VP_Data<core_preview>
{
    VP_DECLARE_PUBLIC(core_preview)
    inline core_previewPrivate(core_preview* parent)
        : VP_Data(parent)
    {
        init();
    }

    void init()
    {
        flag        = 0;
        ms          = 0;
        dstms       = 0;
        bFlag       = false;
        bEnd        = false;
        cb          = nullptr;
        start_time  = 0;
        nIndex      = -1;
        stream      = nullptr;
        frame       = nullptr;
        pFormatCtx  = nullptr;
        ctx         = nullptr;
        m_convert   = nullptr;
        sFile.clear();
        std::thread([=]{ threadCall(); }).detach();
    }

    bool isOk();
    void init(const std::string& file);
    void clear();
    void threadCall();

    int flag;
    std::string src;
    int64_t ms;
    int64_t dstms;
    bool bFlag;
    bool bEnd;

    video_interface* cb;
    int64_t start_time;
    std::string sFile;
    int nIndex;
    AVStream* stream;
    AVFrame* frame;
    AVFormatContext* pFormatCtx;
    AVCodecContext* ctx;
    core_convert* m_convert;
};

bool core_previewPrivate::isOk()
{
    return ctx != nullptr;
}

void core_previewPrivate::init(const std::string& file)
{
    if(bEnd || file == sFile)
        return;

    clear();

    sFile = file;
    frame = av_frame_alloc();
    pFormatCtx = avformat_alloc_context();
    avformat_open_input(&pFormatCtx, file.c_str(), nullptr, nullptr);
    avformat_find_stream_info(pFormatCtx, nullptr);
    for(size_t i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            nIndex = i;
            stream = pFormatCtx->streams[i];
        }
    }
    if(nIndex >= 0)
    {
        start_time = pFormatCtx->start_time;
        ctx = pFormatCtx->streams[nIndex]->codec;
        auto pCodec = avcodec_find_decoder(ctx->codec_id);
        avcodec_open2(ctx, pCodec, nullptr);
        INIT_NEW(&m_convert, core_convert_video)
        m_convert->setContext(ctx);
    }
}

void core_previewPrivate::clear()
{
    sFile.clear();
    nIndex = -1;

    if(frame)
    {
        av_frame_free(&frame);
        frame = nullptr;
    }

    if(ctx)
    {
        avcodec_close(ctx);
        ctx = nullptr;
    }

    if(pFormatCtx)
    {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = nullptr;
    }

    SAFE_RELEASE_PTR(&m_convert)
}

void core_previewPrivate::threadCall()
{
    AVRational aVRational = {1, AV_TIME_BASE};
    AVRational aVRationalMs = {1, AV_TIME_BASE / 1000};
    AVPacket pk;
    int ret = 0;
    int64_t target = 0;
    bool bFirstIFrame = false;

    for(;;)
    {
        if(bEnd)
        {
            break;
        }

        if(!bFlag)
        {
            msleep(10);
            continue;
        }

        do
        {
seek_begin:
            init(src);
            if(!isOk())
            {
                msleep(10);
                break;
            }
            dstms = ms;
            target = dstms * 1000;
            auto targetSeconds = (dstms % 10000) / 1000;
            if(nIndex >= 0)
                target = av_rescale_q(dstms * 1000 + start_time, aVRational, pFormatCtx->streams[nIndex]->time_base);
            if(av_seek_frame(pFormatCtx, nIndex, target, /*AVSEEK_FLAG_BACKWARD*/flag) < 0)
            {
                Log(Log_Warning, "%s:seek img failed!", pFormatCtx->filename);
                break;
            }

            avcodec_flush_buffers(ctx);

//            int count = 0;
            for(;;)
            {
                if(bEnd)
                {
                    break;
                }

                if(ms != dstms)
                {
                    goto seek_begin;
                }

                ret = av_read_frame(pFormatCtx, &pk);
                if(pk.stream_index != nIndex)
                {
                    av_packet_unref(&pk);
                    continue;
                }

                if(!bFirstIFrame && !(pk.flags & AV_PKT_FLAG_KEY))
                {
                    av_packet_unref(&pk);
                    continue;
                }

                bFirstIFrame = true;

//                ++count;
                ret = avcodec_send_packet(ctx, &pk);
                if( ret != 0 && ret != AVERROR(EAGAIN) )
                {
                    av_packet_unref(&pk);
                    continue;
                }

                ret = avcodec_receive_frame(ctx, frame);
                if(ret != 0)
                {
                    av_packet_unref(&pk);
                    continue;
                }
                else
                {
                    auto curDisplay = av_rescale_q(frame->pts, stream->time_base, aVRationalMs);
                    auto curSeconds = (curDisplay % 10000) / 1000;//second alignment
                    if(frame->pts >= target && (!(abs(target - frame->pts) < 100) || targetSeconds != curSeconds))
                        break;
                }

            }

            if(bEnd)
            {
                break;
            }

            av_packet_unref(&pk);
            if(m_convert->convert(frame))
            {
                auto buffer = m_convert->buffer();
                auto ctx    = m_convert->getContext();
                cb->previewDisplayCall(buffer, ctx->width, ctx->height);
            }
//            Log(Log_Debug, "preview!");
//            count = 0;
        }while(0);

        bFlag = false;
    }
}

core_preview::core_preview()
    : VP_INIT(new core_previewPrivate(this))
{
}

core_preview::~core_preview()
{
    VP_D(core_preview);
    d->bEnd = true;
    msleep(10);
    d->clear();
}

void core_preview::setCallBack(video_interface *cb)
{
    VP_D(core_preview);
    d->cb = cb;
}

void core_preview::preview(const std::string& src, int64_t ms, int flag)
{
    VP_D(core_preview);
    d->flag     = flag;
    d->src      = src;
    d->ms       = ms;
    d->bFlag    = true;
}

