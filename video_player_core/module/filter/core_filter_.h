#ifndef CORE_FILTER_H
#define CORE_FILTER_H

#include "video_player_core.h"
#include "../common.h"

class core_filter
{
public:
    core_filter();
    virtual ~core_filter();

public:
    bool init(AVStream* stream, AVCodecContext* pCodecContext);
    void uninit();

    void mix(AVFrame* in);

private:
    AVFilterGraph* graph;
    AVFilterContext* inFilter;
    AVFilterContext* outFilter;

    AVFilterContext* movieCtx;
};

#endif // CORE_FILTER_H
