#include "core_filter_audio.h"
#include "../util/core_util.h"
#include "../media/core_media.h"
#include "../common.h"
#include "Log/Log.h"
#include "core_filter_private.h"

core_filter_audio::core_filter_audio()
{
    outFormat = {{AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE},
                 { AV_CH_LAYOUT_STEREO_DOWNMIX, -1 },
                 { 44100, -1 },
                 100,
                 "1.0",
                 100,
                 "1.0"};
}

core_filter_audio::~core_filter_audio()
{
}

void core_filter_audio::setVol(int nVol)
{
    if(outFormat.volume != nVol)
    {
        outFormat.volume = nVol;
        memset(outFormat.sVolume, 0x00, sizeof(outFormat.sVolume));
        _snprintf(outFormat.sVolume, sizeof(outFormat.sVolume), "%.1f", outFormat.volume / 100.0);
        setFlag(audio_output_flag_volume);
    }
}

void core_filter_audio::setAtempo(int nAtempo)
{
    enum e_atempo_type
    {
        atempo_normal,
        atempo_0_5,
        atempo_1_5,
        atempo_2_0
    };

    if(outFormat.atempo != nAtempo)
    {
        outFormat.atempo = nAtempo;

        double dSpeed = 1.0;
        switch (nAtempo) {
        case atempo_normal:
            break;
        case atempo_0_5:
            dSpeed = 0.5;
            break;
        case atempo_1_5:
            dSpeed = 1.5;
            break;
        case atempo_2_0:
            dSpeed = 2.0;
            break;
        default:
            break;
        }

        memset(outFormat.sAtempo, 0x00, sizeof(outFormat.sAtempo));
        _snprintf(outFormat.sAtempo, sizeof(outFormat.sAtempo), "%.1f", dSpeed);
        setFlag(audio_output_flag_atempo);
    }
}

bool core_filter_audio::initParam(AVStream *stream, AVCodecContext *pCodecContext)
{
    AVRational time_base = stream->time_base;
    char args[256] = {};
    _snprintf(args, sizeof(args),
                "time_base=%d/%d:sample_fmt=%s:sample_rate=%d:channel_layout=0x%llx",
                time_base.num, time_base.den,
                av_get_sample_fmt_name(pCodecContext->sample_fmt),
                pCodecContext->sample_rate,
                pCodecContext->channel_layout);

    m_private->m_sGraphArg = args;

    char args1[256] = {};
    _snprintf(args1, sizeof(args1),
              "aresample=44100,aformat=sample_fmts=s16:channel_layouts=downmix,atempo=%s,volume=%s",
              outFormat.sAtempo,
              outFormat.sVolume
              );

    m_private->m_sFilterDiscription = args1;

    return true;
}

bool core_filter_audio::initFilter()
{
    auto in = avfilter_get_by_name("abuffer");
    auto out = avfilter_get_by_name("abuffersink");

    if(!m_private->createFilter(&m_private->inFilterCtx, in, m_private->m_sGraphArg.c_str(), "in"))
        return false;

    if(!m_private->createFilter(&m_private->outFilterCtx, out, nullptr, "out"))
        return false;

    auto ret = set_int_list(m_private->outFilterCtx, "sample_fmts", outFormat.sample_formats);
    if (ret < 0) {
        Log(Log_Err, "Cannot set output sample format");
        return false;
    }

    ret = set_int_list(m_private->outFilterCtx, "channel_layouts", outFormat.channel_layouts);
    if (ret < 0) {
        Log(Log_Err, "Cannot set output channel layout");
        return false;
    }

    ret = set_int_list(m_private->outFilterCtx, "sample_rates", outFormat.sample_rates);
    if (ret < 0) {
        Log(Log_Err, "Cannot set output sample rate");
        return false;
    }

    return true;
}

void core_filter_audio::update()
{
    if(testFlag(audio_output_flag_atempo))
    {
//        avfilter_graph_send_command(m_private->graph, "atempo", "atempo", "2.0", nullptr, 0, 0);
        Log(Log_Debug, "speed %s", outFormat.sAtempo);
        auto stream = m_private->m_stream;
        auto ctx = m_private->m_codecCtx;
        uninit();
        init(stream, ctx);
    }
    if(testFlag(audio_output_flag_volume))
        avfilter_graph_send_command(m_private->graph, "volume", "volume", outFormat.sVolume, nullptr, 0, 0);
}
