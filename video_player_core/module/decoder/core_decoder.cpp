#include "core_decoder.h"
#include "../util/core_util.h"

core_decoder::core_decoder()
    :nStreamIndex(-1)
    ,format(nullptr)
    ,frame(nullptr)
    ,pCodecContext(nullptr)
    ,pCodec(nullptr)
    ,stream(nullptr)
    ,_clock(0)
{

}

core_decoder::~core_decoder()
{
    uninit();
}

bool core_decoder::init(AVFormatContext *formatCtx, int index)
{
    uninit();
    format = formatCtx;
    nStreamIndex = index;
    if(nStreamIndex < 0)
        return false;
    stream = format->streams[nStreamIndex];
    pCodecContext = stream->codec;
    pCodec = avcodec_find_decoder(pCodecContext->codec_id);
    if(!pCodec)
    {
        Log(Log_Err, "find decoder failed!");
        return false;
    }

    auto ret = avcodec_open2(pCodecContext, pCodec, nullptr);
    if(ret < 0)
    {
        pCodecContext = nullptr;
        Log(Log_Err, "open2 decoder failed(%d)!", ret);
        return false;
    }

    frame = av_frame_alloc();

    Log(Log_Info, "video src size (%d,%d)!", pCodecContext->width, pCodecContext->height);

    return true;
}

void core_decoder::uninit()
{
    if(pCodecContext)
    {
        avcodec_flush_buffers(pCodecContext);
        avcodec_close(pCodecContext);
        pCodecContext = nullptr;
    }

    stream = nullptr;
    pCodec = nullptr;
    if(frame)
    {
        av_frame_free(&frame);
        frame = nullptr;
    }
}

bool core_decoder::check(int index)
{
    return nStreamIndex == index && index >= 0;
}

bool core_decoder::isValid()
{
    return nStreamIndex >= 0;
}

void core_decoder::pushStream(int index)
{
    nStreamIndexs.insert(index);
}

int& core_decoder::index()
{
    return nStreamIndex;
}

void core_decoder::setIndex(int index)
{
    init(format, index);
//    nStreamIndex = index;
}

std::set<int> &core_decoder::indexs()
{
    return nStreamIndexs;
}

unsigned int core_decoder::pktSize()
{
    return pks.size();
}

core_packets &core_decoder::pkts()
{
    return pks;
}

void core_decoder::cleanPkt()
{
    pks.clear();
    if(pCodecContext)
    {
        avcodec_flush_buffers(pCodecContext);
    }
}

double core_decoder::clock()
{
    return _clock;
}

void core_decoder::setClock(double clock)
{
    _clock = clock;
}

bool core_decoder::checkSeekPkt(AVPacket *pk)
{
    static bool bFirst = false;
    if(!bFirst && !(pk->flags & AV_PKT_FLAG_KEY))
    {
        av_packet_unref(pk);
        return true;
    }

    bFirst = true;
    auto ret = avcodec_send_packet(pCodecContext, pk);
    if( ret != 0 && ret != AVERROR(EAGAIN) )
    {
        av_packet_unref(pk);
        return true;
    }

    ret = avcodec_receive_frame(pCodecContext, frame);
    if(ret != 0 /*&& ret != AVERROR(EAGAIN)*/ )
    {
        av_packet_unref(pk);
        return true;
    }

    get_frame_pts(frame);
    bFirst = false;
    return false;
}

double core_decoder::get_frame_pts(AVFrame *frame)
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

    return pts_video;
}
