#ifndef VIDEOFRAME_H
#define VIDEOFRAME_H
#include <memory>
#include <iostream>
#include "video_player_core.h"

#ifdef FRAME_RGB
#define FRAME_LENGTH(x, y) x*y*4
#else
#define FRAME_LENGTH(x, y) x*y*3/2
#endif

class _video_frame_
{
public:
    _video_frame_();
    _video_frame_(unsigned char* buffer, int w, int h);
    ~_video_frame_();

    void set(void* buffer, int w, int h);

    unsigned int W();
    unsigned int H();
    unsigned char* Y();
    unsigned char* U();
    unsigned char* V();

    unsigned int size_y();
    unsigned int size_u();
    unsigned int size_v();
    unsigned int size(int index);
    unsigned char* data(int index, unsigned int& width, unsigned int& height);

private:
    void release();

private:
    unsigned int w;
    unsigned int h;
    unsigned char* framebuffer;
    bool m_bAlloc;
};

typedef std::shared_ptr<_video_frame_> _VideoFramePtr;

#endif // VIDEOFRAME_H
