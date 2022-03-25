#include "videoframe.h"
#include "Log/Log.h"

_video_frame_::_video_frame_()
    : w(0)
    , h(0)
    , framebuffer(nullptr)
    , m_bAlloc(false)
{
}

_video_frame_::_video_frame_(unsigned char* buffer, int w, int h)
    : w(static_cast<unsigned int>(w))
    , h(static_cast<unsigned int>(h))
    , framebuffer(nullptr)
    , m_bAlloc(true)
{
    auto len = static_cast<unsigned int>(FRAME_LENGTH(w,h));
    framebuffer = new unsigned char[len];
    memcpy(framebuffer, buffer, len);
}

_video_frame_::~_video_frame_()
{
    release();
}

void _video_frame_::release()
{
    if(m_bAlloc)
    {
        m_bAlloc = false;
        if(framebuffer)
        {
            delete[] framebuffer;
            framebuffer = nullptr;
        }
    }
}

void _video_frame_::set(void *buffer, int w, int h)
{
    release();
    framebuffer = reinterpret_cast<unsigned char *>(buffer);
    this->w = static_cast<unsigned int>(w);
    this->h = static_cast<unsigned int>(h);
}

unsigned int _video_frame_::W()
{
    return w;
}

unsigned int _video_frame_::H()
{
    return h;
}

unsigned char* _video_frame_::Y()
{
    return framebuffer;
}

unsigned char* _video_frame_::U()
{
    return framebuffer + w * h;
}

unsigned char* _video_frame_::V()
{
    return framebuffer + w * h * 5 / 4;
}

unsigned int _video_frame_::size_y()
{
    return w * h;
}

unsigned int _video_frame_::size_u()
{
    return w * h / 4;
}

unsigned int _video_frame_::size_v()
{
    return w * h / 4;
}

unsigned int _video_frame_::size(int index)
{
    switch (index) {
    case 0:
        return size_y();
    case 1:
        return size_u();
    case 2:
        return size_v();
    }

    return 0;
}

unsigned char* _video_frame_::data(int index, unsigned int& width, unsigned int& height){
    unsigned char* data = nullptr;
    switch (index)
    {
    case 0:
        width = w;
        height = h;
        data = Y();
    break;
    case 1:
        width = w / 2;
        height = h / 2;
        data = U();
    break;
    case 2:
        width = w / 2;
        height = h / 2;
        data = V();
    break;
    }

    return data;
}
