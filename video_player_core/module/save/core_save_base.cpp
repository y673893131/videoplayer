#include "core_save_base.h"
#include "../util/core_util.h"
#include "../media/core_media.h"
#include "../common.h"
#include "Log/Log.h"

core_save_base::core_save_base()
    :m_format(nullptr)
    ,m_lastDts(0)
{
}

core_save_base::~core_save_base()
{
    uninit();
}

bool core_save_base::init(AVFormatContext* pFormat, int nIndex)
{
    uninit();

    m_sFileName = outoutFile();

    auto pGuess = guess();
    if(!pGuess)
        return false;

    auto ret = avformat_alloc_output_context2(&m_format, pGuess, nullptr, m_sFileName.c_str());
    if(ret < 0 || !m_format)
        return false;

    if(!initStream(pFormat, nIndex))
        return false;

    if(!initHeader())
        return false;

    return true;
}

void core_save_base::uninit()
{
    if(m_format)
    {
        if(!(m_format->flags & AVFMT_NOFILE))
        {
            avio_close(m_format->pb);
        }

        avformat_free_context(m_format);
        m_format = nullptr;
    }

    m_lastDts = 0;
}

bool core_save_base::initStream(AVFormatContext* pFormat, int nIndex)
{
    auto in = pFormat->streams[nIndex];
    m_in = in;
    m_stream = avformat_new_stream(m_format, in->codec->codec);
    auto ret = avcodec_parameters_copy(m_stream->codecpar, in->codecpar);
    if(ret < 0)
        return false;
    ret = avcodec_copy_context(m_stream->codec, in->codec);
    if(ret < 0)
        return false;

    m_stream->time_base = in->time_base;
    m_stream->codecpar->codec_tag = 0;
    m_stream->codec->codec_tag = 0;
    if(m_format->oformat->flags & AVFMT_GLOBALHEADER)
    {
        m_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }


    av_dump_format(m_format, 0, m_sFileName.c_str(), 1);
    return true;
}

bool core_save_base::initHeader()
{
    int ret = 0;
    if(!(m_format->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&m_format->pb, m_sFileName.c_str(), AVIO_FLAG_WRITE);
        if(ret < 0)
        {
            char buf[1024] = {};
            av_strerror(ret, buf, 1024);
            Log(Log_Warning, "avformat_write_header failed, %s", buf);
            return false;
        }
    }

    ret = avformat_write_header(m_format, nullptr);
    if(ret < 0)
    {
        char buf[1024] = {};
        av_strerror(ret, buf, 1024);
        Log(Log_Warning, "avformat_write_header failed, %s", buf);

        return false;
    }

    return true;
}

void core_save_base::rescalePacket(AVPacket* pk)
{
    if(pk->pts != AV_NOPTS_VALUE)
    {
        pk->pts = av_rescale_q_rnd(pk->pts, m_in->time_base, m_stream->time_base,
            (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    }

    if(pk->dts != AV_NOPTS_VALUE)
    {
        pk->dts = av_rescale_q_rnd(pk->dts, m_in->time_base, m_stream->time_base,
            (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    }

    pk->duration = av_rescale_q(pk->duration, m_in->time_base, m_stream->time_base);
    pk->pos = -1;
    pk->stream_index = 0;

    if(pk->dts <= m_lastDts)
    {
        ++pk->dts;
        ++pk->pts;
    }

    m_lastDts = pk->dts;
}

bool core_save_base::start(AVFormatContext* pFormat, int nIndex)
{
    return init(pFormat, nIndex);
}

void core_save_base::stop()
{
    if(m_format)
    {
        av_write_trailer(m_format);
    }

    uninit();
}
