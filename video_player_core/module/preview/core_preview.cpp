#include "core_preview.h"
#include "../util/core_util.h"

core_preview::core_preview()
{
    sFile.clear();
    nIndex = -1;
    frame = nullptr;
    pFormatCtx = nullptr;
    ctx = nullptr;
    outFrame = nullptr;
}

core_preview::~core_preview()
{
    clear();
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
            nIndex = i;
    }
    if(nIndex >= 0)
    {
        ctx = pFormatCtx->streams[nIndex]->codec;
        auto pCodec = avcodec_find_decoder(ctx->codec_id);
        avcodec_open2(ctx, pCodec, nullptr);
        outFrame = new core_frame_convert(ctx, AV_PIX_FMT_RGB24);
    }
}

void core_preview::preview(const std::string& src, int64_t ms, video_interface* cb)
{
    Log(Log_Info, "%I64d [thread_id]:%d", ms, core_util::getThreadId());
    init(src);
    if(!isOk())
        return;
    auto target = ms * 1000;
    AVRational aVRational = {1, AV_TIME_BASE};
    if(nIndex >= 0)
        target = av_rescale_q(target, aVRational, pFormatCtx->streams[nIndex]->time_base);
    if(av_seek_frame(pFormatCtx, nIndex, target, AVSEEK_FLAG_BACKWARD) < 0)
    {
        Log(Log_Warning, "%s:seek img failed!", pFormatCtx->filename);
        return;
    }

    avcodec_flush_buffers(ctx);
    AVPacket pk;
    int ret = 0;
    bool bFirstIFrame = false;
    while(1)
    {
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
            break;
        }
    }

    std::string sData = "seek:" + std::to_string(pk.pts) + "\n";
    OutputDebugStringA(sData.c_str());
    av_packet_unref(&pk);
    outFrame->scalePreview(frame, cb);
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
