#include "core_filter_private.h"
#include "../util/core_util.h"
#include "../media/core_media.h"
#include "../common.h"
#include "Log/Log.h"

core_filter_private::core_filter_private()
    : m_stream(nullptr)
    , m_codecCtx(nullptr)
    , graph(nullptr)
    , inFilterCtx(nullptr)
    , outFilterCtx(nullptr)
    , inputs(nullptr)
    , outputs(nullptr)
    , m_frame(nullptr)
{
}

core_filter_private::~core_filter_private()
{
    uninit();
}

bool core_filter_private::init(AVStream* stream, AVCodecContext* pCodecContext)
{
    uninit();
    avfilter_register_all();

    m_stream = stream;
    m_codecCtx = pCodecContext;

    graph = avfilter_graph_alloc();

    m_frame = av_frame_alloc();
    return true;
}

void core_filter_private::uninit()
{
    inFilterCtx = nullptr;
    outFilterCtx = nullptr;
    if(graph)
    {
        avfilter_graph_free(&graph);
        graph = nullptr;
    }
    if(m_frame)
    {
        av_frame_free(&m_frame);
        m_frame = nullptr;
    }

    if(inputs)
    {
        avfilter_inout_free(&inputs);
        inputs = nullptr;
    }

    if(outputs)
    {
        avfilter_inout_free(&outputs);
        outputs = nullptr;
    }
}

bool core_filter_private::link()
{
    //in out
    outputs = avfilter_inout_alloc();
    inputs  = avfilter_inout_alloc();

    outputs->name       = av_strdup("in");
    outputs->filter_ctx = inFilterCtx;
    outputs->pad_idx    = 0;
    outputs->next       = nullptr;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = outFilterCtx;
    inputs->pad_idx    = 0;
    inputs->next       = nullptr;

    if (avfilter_graph_parse_ptr(
                graph,
                m_sFilterDiscription.c_str(),
                &inputs, &outputs, nullptr) < 0)
    {
        return false;
    }

    auto ret = avfilter_graph_config(graph, nullptr);
    if(ret < 0)
        return false;

    return true;
}

AVFrame *core_filter_private::mix(AVFrame *in)
{
    if(!inFilterCtx || !outFilterCtx)
        return nullptr;
    int ret = av_buffersrc_add_frame(inFilterCtx, in);
    if(ret < 0)
    {
        Log(Log_Err, "av_buffersrc_add_frame failed[%d]", ret);
        return nullptr;
    }

    ret = av_buffersink_get_frame(outFilterCtx, m_frame);
    if(ret < 0)
    {
        Log(Log_Err, "av_buffersink_get_frame failed[%d]", ret);
        return nullptr;
    }

    return m_frame;
}

bool core_filter_private::createFilter(AVFilterContext **filterCtx, const AVFilter *filter, const char* arg, const char *name)
{
    *filterCtx = avfilter_graph_alloc_filter(graph, filter, name);
    if(!*filterCtx)
        return false;

    auto ret = avfilter_init_str(*filterCtx, arg);
    if(ret < 0)
        return false;

//    auto ret = avfilter_graph_create_filter(filterCtx, filter, name, arg, nullptr, graph);
//    if(ret < 0)
//        return false;
    return true;
}
