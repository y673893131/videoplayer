#ifndef CORE_FILTER_VIDEO_H
#define CORE_FILTER_VIDEO_H

#include "video_player_core.h"
#include "../common.h"
#include "core_filter_base.h"

enum video_output_flag
{
    video_output_flag_scale
};

struct video_output_format
{
    AVPixelFormat pixel_formats[2];
    int w;
    int h;
    char scale[128];
    unsigned int size;
    uint8_t* buffer;
};

class core_filter_video : public core_filter_base
{
public:
    core_filter_video();
    ~core_filter_video() override;

    void setScale(int w, int h);
    int width();
    int height();
    uint8_t* buffer();

public:
    AVFrame * mix(AVFrame *in) override;

private:
    bool initParam(AVStream *stream, AVCodecContext *pCodecContext) override;
    bool initFilter() override;

    void update() override;
private:
    video_output_format outFormat;
};

#endif // CORE_FILTER_VIDEO_H
