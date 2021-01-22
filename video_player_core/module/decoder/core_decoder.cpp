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
    pCodec = avcodec_find_decoder(stream->codecpar->codec_id);
    pCodecContext = avcodec_alloc_context3(pCodec);
    if(!pCodecContext)
    {
        Log(Log_Err, "avcodec_alloc_context3 failed!");
        return false;
    }

    avcodec_parameters_to_context(pCodecContext, stream->codecpar);
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

bool core_decoder::decode(AVPacket */*pk*/)
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
