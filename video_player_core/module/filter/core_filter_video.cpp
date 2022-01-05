#include "core_filter_video.h"
#include "../util/core_util.h"
#include "../media/core_media.h"
#include "../common.h"
#include "Log/Log.h"
#include "core_filter_private.h"

core_filter_video::core_filter_video()
{
    outFormat = {{AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE},
                 1,
                 1,
                 "1:1",
                 0,
                 nullptr};
}

core_filter_video::~core_filter_video()
{
}

void core_filter_video::setScale(int w, int h)
{
    w = FFALIGN(w, 64);
    h = FFALIGN(h, 64);
    if(outFormat.w != w || outFormat.h != h)
    {
        outFormat.w = w;
        outFormat.h = h;
        memset(outFormat.scale, 0x00, sizeof(outFormat.scale));
        _snprintf(outFormat.scale, sizeof(outFormat.scale),
                  "scale=%d:%d",
                  w,
                  h);

        if(outFormat.buffer)
        {
            av_free(outFormat.buffer);
            outFormat.buffer = nullptr;
        }

        outFormat.size = static_cast<unsigned int>(av_image_get_buffer_size(outFormat.pixel_formats[0], w, h, 1));
        outFormat.buffer = reinterpret_cast<uint8_t*>(av_malloc(outFormat.size * sizeof(uint8_t)));
        av_image_fill_arrays(m_private->m_frame->data, m_private->m_frame->linesize, outFormat.buffer, outFormat.pixel_formats[0], w, h, 1);
//        avfilter_graph_send_command(m_private->graph, "scale", "scale", outFormat.scale, nullptr, 0, 0);
//        setFlag(video_output_flag_scale);
    }
}

int core_filter_video::width()
{
    return outFormat.w;
}

int core_filter_video::height()
{
    return outFormat.h;
}

uint8_t *core_filter_video::buffer()
{
    return outFormat.buffer;
}

bool core_filter_video::initParam(AVStream *stream, AVCodecContext *pCodecContext)
{
    char args[256] = {};
    _snprintf(args, sizeof(args),
                "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                pCodecContext->width,
                pCodecContext->height,
                pCodecContext->pix_fmt,
                stream->time_base.num, stream->time_base.den,
                stream->codecpar->sample_aspect_ratio.num,
                stream->codecpar->sample_aspect_ratio.den);

    m_private->m_sGraphArg = args;

    setScale(pCodecContext->width, pCodecContext->height);
    char args1[256] = {};
    _snprintf(args1, sizeof(args1),
              "%s",
              outFormat.scale
              );

    m_private->m_sFilterDiscription = args1;

    return true;
}

bool core_filter_video::initFilter()
{
    auto in = avfilter_get_by_name("buffer");
    auto out = avfilter_get_by_name("buffersink");

    if(!m_private->createFilter(&m_private->inFilterCtx, in, m_private->m_sGraphArg.c_str(), "in"))
        return false;

    if(!m_private->createFilter(&m_private->outFilterCtx, out, nullptr, "out"))
        return false;

    auto ret = set_int_list(m_private->outFilterCtx, "pix_fmts", outFormat.pixel_formats);
    if (ret < 0) {
        Log(Log_Err, "Cannot set output pix format");
        return false;
    }

    return true;
}

AVFrame *core_filter_video::mix(AVFrame *in)
{
    auto frame = core_filter_base::mix(in);
    if(frame)
    {
//        int ret = av_image_fill_arrays(frame->data, frame->linesize, outFormat.buffer, outFormat.pixel_formats[0], outFormat.w, outFormat.h, 1);
        auto length = frame->linesize[0] * frame->height;
        auto index = 0;
        memcpy(outFormat.buffer + index, frame->data[0], length);
        index += length;
        length /= 4;
        memcpy(outFormat.buffer + index, frame->data[1], length);
        index += length;
        memcpy(outFormat.buffer + index, frame->data[2], length);
    }

    return frame;
}

void core_filter_video::update()
{
    if(testFlag(video_output_flag_scale))
        avfilter_graph_send_command(m_private->graph, "scale", "scale", outFormat.scale, nullptr, 0, 0);
}
