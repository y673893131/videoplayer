#include "core_decoder.h"
#include "../util/core_util.h"

core_decoder::core_decoder()
    :nStreamIndex(-1)
    ,format(nullptr)
    ,frame(nullptr)
    ,pCodecContext(nullptr)
    ,pCodec(nullptr)
    ,stream(nullptr)
    ,pts(0)
    ,m_filter(nullptr)
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
    pCodec = avcodec_find_decoder(stream->codecpar->codec_id);
    pCodecContext = avcodec_alloc_context3(pCodec);
    if(!pCodecContext)
    {
        Log(Log_Err, "avcodec_alloc_context3 failed!");
        return false;
    }

    avcodec_parameters_to_context(pCodecContext, stream->codecpar);
    pCodecContext->pkt_timebase = stream->time_base;
    pCodecContext->thread_count = 4;
    pCodecContext->thread_type = FF_THREAD_FRAME;
    if(pCodecContext->channel_layout <= 0)
        pCodecContext->channel_layout = static_cast<uint64_t>(av_get_default_channel_layout(pCodecContext->channels));

    auto ret = avcodec_open2(pCodecContext, pCodec, nullptr);
    if(ret < 0)
    {
        pCodecContext = nullptr;
        Log(Log_Err, "avcodec_open2 failed(%d)!", ret);
        return false;
    }

    frame = av_frame_alloc();
    return true;
}

void core_decoder::uninit()
{
    if(pCodecContext)
    {
        avcodec_flush_buffers(pCodecContext);
        avcodec_free_context(&pCodecContext);
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

bool core_decoder::checkSeekPkt(AVPacket *)
{
    return false;
}

bool core_decoder::check(int index)
{
    return nStreamIndex == index && index >= 0;
}

bool core_decoder::isValid()
{
    return nStreamIndex >= 0;
}

void core_decoder::pushStream(unsigned int index)
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
}

std::set<unsigned int> &core_decoder::indexs()
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

void core_decoder::flush()
{
    if(pCodecContext)
    {
        avcodec_flush_buffers(pCodecContext);
    }
}

int64_t core_decoder::clock()
{
    return pts;
}

int64_t core_decoder::displayClock()
{
    return getDisplayPts(pts);
}

void core_decoder::setClock(int64_t pts)
{
    this->pts = pts;
}

void core_decoder::calcClock(AVPacket* pk)
{
    int64_t pts = 0;

    if(frame->pts != AV_NOPTS_VALUE)
        pts = frame->pts;
    else if(frame->pkt_dts != AV_NOPTS_VALUE)
        pts = frame->pkt_dts;
    else
    {
        if(frame->opaque)
        {
            auto opa = *reinterpret_cast<int64_t*>(frame->opaque);
            if(opa != AV_NOPTS_VALUE)
                pts = opa;
            else
                pts = 0;
        }
        else
        {
            pts = 0;
        }
    }

    checkQSVClock(pk, pts);

//    pts *= av_q2d(stream->time_base);
    setClock(pts);
}

void core_decoder::checkQSVClock(AVPacket *, int64_t& /*pts*/)
{
}

int64_t core_decoder::getInteralPts(int64_t pos)
{
    if(stream)
    {
        static AVRational aVRational = {1, AV_TIME_BASE};
        return av_rescale_q(pos, aVRational, stream->time_base);
    }
    else
    {
        return pos;
    }
}

int64_t core_decoder::getDisplayPts(int64_t pos)
{
    if(stream)
    {
        static AVRational aVRational = {1, AV_TIME_BASE / 1000};
        return av_rescale_q(pos, stream->time_base, aVRational);
    }
    else
    {
        return pos / 1000;
    }
}
