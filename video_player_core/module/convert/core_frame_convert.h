#ifndef CORE_FRAME_CONVERT_H
#define CORE_FRAME_CONVERT_H

#include "../common.h"
#include "../lock/core_graud_lock.h"
#include "video_player_core.h"

class core_frame_convert
{
public:
    core_frame_convert();
    core_frame_convert(AVCodecContext* codec, AVPixelFormat format=AV_PIX_FMT_YUV420P);
    virtual ~core_frame_convert();

    void setSize(int w, int h);
    int width();
    int height();
    void setsrcCodec(AVCodecContext* codec);
    void init(int width = 0, int height = 0);
    void uninit();
    void initFrame();
    void scale(AVFrame* src, void* cb);
    void scalePreview(AVFrame* src, void* cb);
private:
    AVFrame* frame;
    uint8_t* buffer;
    SwsContext* swsCov;
    AVCodecContext* srcCodec;
    core_graud_lock mutex;
    size_t size;
    int w;
    int h;
    int srcWidth;
    int srcHeight;
    AVPixelFormat picflag;
};

#endif // CORE_FRAME_CONVERT_H
