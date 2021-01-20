#include "core_audio_sample.h"

core_audio_sample::core_audio_sample()
    :channels(2)
    ,fmt(AV_SAMPLE_FMT_S16)
    ,rate(44100)
{
    layout = av_get_default_channel_layout(channels);
    layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
}

core_audio_sample::core_audio_sample(const AVCodecContext *ctx)
    :channels(ctx->channels)
    ,fmt(ctx->sample_fmt)
    ,rate(ctx->sample_rate)
    ,layout(ctx->channel_layout)
{
    if(layout <= 0)
    {
        int ch_layout = static_cast<int>(layout);
        layout = av_get_default_channel_layout(ch_layout);
    }
}

int core_audio_sample::linesize()
{
    return rate * channels;
}
