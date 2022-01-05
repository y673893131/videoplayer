#ifndef CORE_FILTER_AUDIO_H
#define CORE_FILTER_AUDIO_H

#include "video_player_core.h"
#include "../common.h"
#include "core_filter_base.h"

enum audio_output_flag
{
    audio_output_flag_volume,
    audio_output_flag_atempo
};

struct audio_output_format
{
    AVSampleFormat sample_formats[3];
    int64_t channel_layouts[2];
    int sample_rates[2];
    int atempo;
    char sAtempo[20];
    int volume;
    char sVolume[20];
};

class core_filter_audio : public core_filter_base
{
public:
    core_filter_audio();
    ~core_filter_audio() override;

    void setVol(int nVol);
    void setAtempo(int nAtempo);
private:
    bool initParam(AVStream *stream, AVCodecContext *pCodecContext) override;
    bool initFilter() override;
    void update() override;

private:
    audio_output_format outFormat;
};

#endif // CORE_FILTER_AUDIO_H
