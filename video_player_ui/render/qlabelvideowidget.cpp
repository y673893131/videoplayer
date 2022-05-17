#include "qlabelvideowidget.h"
#include <QDebug>
QLabelVideoWidget::QLabelVideoWidget(QWidget *parent)
    : QLabel(parent)
{

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

//void QLabelVideoWidget::displayCall(void *data, int width, int height)
//{
//    unsigned int w = static_cast<unsigned int>(width);
//    unsigned int h = static_cast<unsigned int>(height);
//    char* rgb = new char[w * h * 3];
//    memset(rgb, 0x00, w * h * 3);
//    auto _data = reinterpret_cast<unsigned char*>(data);
//    auto _rgb = reinterpret_cast<unsigned char*>(rgb);
//    auto _rgb0 = reinterpret_cast<uchar*>(rgb);
////    YUV420_2_RGB((unsigned char*)data, (unsigned char*)rgb, width, height);
//    yuv420p_to_rgb24(_data, _rgb, width, height);
////    m_core->_cov(data, rgb, width, height, width * height * 3);
//    QImage img(_rgb0, width, height, QImage::Format_RGB888);
//    this->setPixmap(QPixmap::fromImage(img).scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
////    m_img->setScaledContents(true);
//    delete[] rgb;
//}
