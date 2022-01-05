#ifndef CORE_FILTER_PRIVATE_H
#define CORE_FILTER_PRIVATE_H

#include "video_player_core.h"
#include "../common.h"

#define set_int_list(filter, name, value) av_opt_set_int_list(\
        filter,\
        name,\
        value,\
        -1,\
        AV_OPT_SEARCH_CHILDREN)

class core_filter_private
{
public:
    core_filter_private();
    ~core_filter_private();

    bool init(AVStream* stream, AVCodecContext* pCodecContext);
    void uninit();

    bool createFilter(AVFilterContext **filterCtx, const AVFilter *filter, const char* arg, const char *name);
    bool link();

    AVFrame* mix(AVFrame* in);
public:
    AVStream* m_stream;
    AVCodecContext* m_codecCtx;

    std::string m_sBegFilter;
    std::string m_sEndFilter;
    std::string m_sGraphArg;
    std::string m_sFilterDiscription;

    AVFilterGraph* graph;
    AVFilterContext* inFilterCtx;
    AVFilterContext* outFilterCtx;

    AVFilterInOut *inputs;
    AVFilterInOut *outputs;

    AVFrame* m_frame;
};

#endif // CORE_FILTER_PRIVATE_H
