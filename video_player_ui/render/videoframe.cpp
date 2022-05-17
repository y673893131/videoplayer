#include "videoframe.h"
#include "Log/Log.h"
#include "video_player_core.h"
#include <QByteArray>

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

static long int crv_tab[256];
static long int cbu_tab[256];
static long int cgu_tab[256];
static long int cgv_tab[256];
static long int tab_76309[256];
static unsigned char clp[1024];   //for clip in CCIR601

void init_yuv420p_table()
{
    long int crv,cbu,cgu,cgv;
    int i,ind;
    static int init = 0;

    if (init == 1) return;

    crv = 104597; cbu = 132201;  /* fra matrise i global.h */
    cgu = 25675;  cgv = 53279;

    for (i = 0; i < 256; i++)
    {
        crv_tab[i] = (i-128) * crv;
        cbu_tab[i] = (i-128) * cbu;
        cgu_tab[i] = (i-128) * cgu;
        cgv_tab[i] = (i-128) * cgv;
        tab_76309[i] = 76309*(i-16);
    }

    for (i = 0; i < 384; i++)
        clp[i] = 0;
    ind = 384;
    for (i = 0;i < 256; i++)
        clp[ind++] = static_cast<unsigned char>(i);
    ind = 640;
    for (i = 0;i < 384; i++)
        clp[ind++] = 255;

    init = 1;
}

void _video_frame_::yuv420p_to_rgb24(/*YUV_TYPE type, */unsigned char* yuvbuffer,unsigned char* rgbbuffer, int width,int height)
{
    int y1, y2, u, v;
    unsigned char *py1, *py2;
    int i, j, c1, c2, c3, c4;
    unsigned char *d1, *d2;
    unsigned char *src_u, *src_v;
//    static int init_yuv420p = 0;

    src_u = yuvbuffer + width * height;   // u
    src_v = src_u + width * height / 4;  //  v

//    if (type == FMT_YV12)
//    {
//        src_v = yuvbuffer + width * height;   // v
//        src_u = src_u + width * height / 4;  //  u
//    }
    py1 = yuvbuffer;   // y
    py2 = py1 + width;
    d1 = rgbbuffer;
    d2 = d1 + 3 * width;

    init_yuv420p_table();

    for (j = 0; j < height; j += 2)
    {
        for (i = 0; i < width; i += 2)
        {
            u = *src_u++;
            v = *src_v++;

            c1 = crv_tab[v];
            c2 = cgu_tab[u];
            c3 = cgv_tab[v];
            c4 = cbu_tab[u];

            //up-left
            y1 = tab_76309[*py1++];
            *d1++ = clp[384+((y1 + c1)>>16)];
            *d1++ = clp[384+((y1 - c2 - c3)>>16)];
            *d1++ = clp[384+((y1 + c4)>>16)];

            //down-left
            y2 = tab_76309[*py2++];
            *d2++ = clp[384+((y2 + c1)>>16)];
            *d2++ = clp[384+((y2 - c2 - c3)>>16)];
            *d2++ = clp[384+((y2 + c4)>>16)];

            //up-right
            y1 = tab_76309[*py1++];
            *d1++ = clp[384+((y1 + c1)>>16)];
            *d1++ = clp[384+((y1 - c2 - c3)>>16)];
            *d1++ = clp[384+((y1 + c4)>>16)];

            //down-right
            y2 = tab_76309[*py2++];
            *d2++ = clp[384+((y2 + c1)>>16)];
            *d2++ = clp[384+((y2 - c2 - c3)>>16)];
            *d2++ = clp[384+((y2 + c4)>>16)];
        }
        d1  += 3*width;
        d2  += 3*width;
        py1 += width;
        py2 += width;
    }
}

QImage _video_frame_::toImage()
{
    return toImage(Y(), static_cast<int>(w), static_cast<int>(h));
}

QImage _video_frame_::toImage(void *data, int width, int height)
{
#ifdef FRAME_RGB
    QImage img(static_cast<unsigned char*>(data), static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB888);
#else
    QImage img(QSize(width, height), QImage::Format_RGB888);
    yuv420p_to_rgb24(static_cast<unsigned char*>(data), img.bits(), width, height);
#endif

    return img;
}
