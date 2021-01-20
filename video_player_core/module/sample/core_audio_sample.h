#ifndef CORE_AUDIO_SAMPLE_H
#define CORE_AUDIO_SAMPLE_H

#include "../common.h"

class core_sdl_op;
class core_audio_sample
{
public:
    core_audio_sample();
    core_audio_sample(const AVCodecContext* ctx);

    int linesize();

private:
    int channels;
    AVSampleFormat fmt;
    int rate;
    uint64_t layout;

    friend class core_sdl_op;
};

#endif // CORE_AUDIO_SAMPLE_H
