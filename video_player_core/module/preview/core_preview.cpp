#include "core_preview.h"
#include "../util/core_util.h"

core_preview::core_preview()
{
    flag = 0;
    ms = 0;
    dstms = 0;

    bFlag = false;
    bEnd = false;
    cb = nullptr;
    start_time = 0;
    sFile.clear();
    nIndex = -1;
    stream = nullptr;
    frame = nullptr;
    pFormatCtx = nullptr;
    ctx = nullptr;
    outFrame = nullptr;

    std::thread([=]{ threadCall(); }).detach();
}

core_preview::~core_preview()
{
    bEnd = true;
    msleep(10);
    clear();
}

void core_preview::setCallBack(video_interface *cb)
{
    this->cb = cb;
}

bool core_preview::isOk()
{
    return ctx != nullptr;
}

void core_preview::init(const std::string& file)
{
    if(file == sFile)
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
        outFrame = new core_frame_convert(ctx, AV_PIX_FMT_RGB24);
    }
}

void core_preview::preview(const std::string& src, int64_t ms, int flag)
{
    this->flag = flag;
    this->src = src;
    this->ms = ms;
    bFlag = true;
}

void core_preview::clear()
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

    SAFE_RELEASE_PTR(&outFrame)
}

void core_preview::threadCall()
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
            outFrame->scalePreview(frame, cb);
//            Log(Log_Debug, "preview:[frames=%d]", count);
//            count = 0;
        }while(0);

        bFlag = false;
    }
}
