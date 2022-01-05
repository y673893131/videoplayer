#include "core_filter.h"
#include "../util/core_util.h"
#include "../media/core_media.h"
#include "../common.h"
#include "Log/Log.h"

core_filter::core_filter()
    : graph(nullptr)
    , inFilter(nullptr)
    , outFilter(nullptr)
    , movieCtx(nullptr)
{
}

core_filter::~core_filter()
{

}

bool core_filter::init(AVStream* stream, AVCodecContext* pCodecContext)
{
    uninit();
    avfilter_register_all();

    const AVFilter* pp = nullptr;
    while(1)
    {
        pp = avfilter_next(pp);
        if(!pp)
            break;
        LogB(Log_Info, "[filter] %s", pp->name);
    }
    auto in = avfilter_get_by_name("buffer");
    auto out = avfilter_get_by_name("buffersink");
    auto movie = avfilter_get_by_name("movie");
    auto framerate = avfilter_get_by_name("framerate");
    auto overlay = avfilter_get_by_name("overlay");


    graph = avfilter_graph_alloc();
    if(!graph)
        return false;

    char args[256];
    _snprintf(args, sizeof(args),
                "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                pCodecContext->width,
                pCodecContext->height,
                pCodecContext->pix_fmt,
                stream->time_base.num, stream->time_base.den,
                stream->codecpar->sample_aspect_ratio.num,
                stream->codecpar->sample_aspect_ratio.den);

    auto ret = avfilter_graph_create_filter(&inFilter, in, "in", args, nullptr, graph);
    if(ret < 0)
        return false;

    AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
    AVBufferSinkParams* sink_params = av_buffersink_params_alloc();
    sink_params->pixel_fmts = pix_fmts;
    ret = avfilter_graph_create_filter(&outFilter, out, "out", nullptr, sink_params, graph);
    if(ret < 0)
        return false;

    ret = avfilter_graph_create_filter(&movieCtx, movie, "_logo", "sex_boy.png", nullptr, graph);
    if (ret < 0)
        return false;

//    const AVOption *opt0 = av_opt_find(movieCtx->priv, "_logo", NULL, 0, 0);
//    AVOption opt1 = {};
//    sprintf(opt1, "0x%06X", color & 0x00FFFFFF);
//    ret = av_opt_set(movieCtx->priv, "_logo", "sex_girl.png", 0);

    AVFilterContext* overlayCtx = nullptr;
    ret = avfilter_graph_create_filter(&overlayCtx, overlay, "overlay", "x=100:y=100", nullptr, graph);
    if (ret < 0)
        return false;

    ret = avfilter_link(inFilter, 0, overlayCtx, 0);
    if (ret < 0)
        return false;

    ret = avfilter_link(movieCtx, 0, overlayCtx, 1);
    if (ret < 0)
        return false;

    ret = avfilter_link(overlayCtx, 0, outFilter, 0);
    if (ret < 0)
        return false;


    ret = avfilter_graph_config(graph, nullptr);
    if(ret < 0)
        return false;

    return true;
}

void core_filter::uninit()
{
    inFilter = nullptr;
    outFilter = nullptr;
    if(graph)
    {
        avfilter_graph_free(&graph);
        graph = nullptr;
    }
}

void core_filter::mix(AVFrame* in)
{
    if(!inFilter || !outFilter)
        return;
    int ret = av_buffersrc_add_frame(inFilter, in);
    if(ret)
    {
        Log(Log_Err, "av_buffersrc_add_frame failed[%d]", ret);
        return;
    }

    ret = av_buffersink_get_frame(outFilter, in);
    if(ret)
    {
        Log(Log_Err, "av_buffersink_get_frame failed[%d]", ret);
        return;
    }
}
