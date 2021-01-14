#include "qlabelvideowidget.h"
#include <QDebug>
QLabelVideoWidget::QLabelVideoWidget(QWidget *parent)
    : QLabel(parent)
{

}


void QLabelVideoWidget::totalTime(const int64_t t)
{
    qDebug() << "label total: " << t;
}

void YUV420_2_RGB(unsigned char* pYUV, unsigned char* pRGB, int width, int height)
{
    unsigned char* pY = pYUV;
    unsigned char* pU = pYUV + height*width;
    unsigned char* pV = pU + (height*width / 4);


    unsigned char* pBGR = nullptr;
    unsigned char R = 0;
    unsigned char G = 0;
    unsigned char B = 0;
    unsigned char Y = 0;
    unsigned char U = 0;
    unsigned char V = 0;
    double temp = 0;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            pBGR = pRGB + i*width * 3 + j * 3;

            Y = *(pY + i*width + j);
            U = *pU;
            V = *pV;
/*
        vec3 yuv; \
        vec3 rgb; \
        yuv.x = texture2D(tex_y, textureOut).r; \
        yuv.x = 1.164*(yuv.x - 0.0625);"
        "yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
        yuv.z = texture2D(tex_v, textureOut).r - 0.5;"
        "rgb.x = yuv.x + 1.596023559570*yuv.z;\
        rgb.y = yuv.x - 0.3917694091796875*yuv.y - 0.8129730224609375*yuv.z;\
        rgb.z = yuv.x + 2.017227172851563*yuv.y;"
*/

//            float in[3];
//            in[0] = Y/256.0;
//            in[0] = 1.164*(in[0] - 0.0625)*256.0;
//            in[1] = (U/256.0 - 0.5)*256.0;
//            in[2] = (V/256.0 - 0.5)*256.0;
//            R = in[0] + 1.596023559570 * in[2];
//            G = in[0] - 0.3917694091796875 * in[1] - 0.8129730224609375*in[2];
//            B = in[0] + 2.017227172851563 * in[1];

            //yuv -> rgb
            temp = Y + ((1.779) * (U - 128));
            R = temp<0 ? 0 : (temp>255 ? 255 : static_cast<unsigned char>(temp));

            temp = (Y - (0.3455) * (U - 128) - (0.7169) * (V - 128));
            G = temp<0 ? 0 : (temp>255 ? 255 : static_cast<unsigned char>(temp));

            temp = (Y + (1.4075)*(V - 128));
            B = temp<0 ? 0 : (temp>255 ? 255 : static_cast<unsigned char>(temp));

            *pBGR = B;
            *(pBGR + 1) = G;
            *(pBGR + 2) = R;

            if (j % 2 != 0)
            {
                pU++;
                pV++;
            }

        }
        if (i % 2 == 0)
        {
            pU = pU - width / 2;
            pV = pV - width / 2;
        }
    }
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

void yuv420p_to_rgb24(/*YUV_TYPE type, */unsigned char* yuvbuffer,unsigned char* rgbbuffer, int width,int height)
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

void QLabelVideoWidget::displayCall(void *data, int width, int height)
{
    unsigned int w = static_cast<unsigned int>(width);
    unsigned int h = static_cast<unsigned int>(height);
    char* rgb = new char[w * h * 3];
    memset(rgb, 0x00, w * h * 3);
    auto _data = reinterpret_cast<unsigned char*>(data);
    auto _rgb = reinterpret_cast<unsigned char*>(rgb);
    auto _rgb0 = reinterpret_cast<uchar*>(rgb);
//    YUV420_2_RGB((unsigned char*)data, (unsigned char*)rgb, width, height);
    yuv420p_to_rgb24(_data, _rgb, width, height);
//    m_core->_cov(data, rgb, width, height, width * height * 3);
    QImage img(_rgb0, width, height, QImage::Format_RGB888);
    this->setPixmap(QPixmap::fromImage(img).scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
//    m_img->setScaledContents(true);
    delete[] rgb;
}

void QLabelVideoWidget::previewDisplayCall(void *data, int width, int height)
{

}

void QLabelVideoWidget::endCall()
{
}
