#include "qrenderprivate.h"
#include "ui/tool/menu/qplaymenu.h"
#include <QDebug>

QRenderPrivate::QRenderPrivate()
    : m_frame(nullptr)
    , m_freq(nullptr)
    , m_freqLast(nullptr)
    , m_freqCount(0)
    , m_video(0, 0)
    , m_window(0, 0)
    , m_fScaleX(1.0f)
    , m_fScaleY(1.0f)
    , m_scaleType(0)
{
}

void QRenderPrivate::setVideoSize(int w, int h)
{
    m_video = QSize(w, h);
}

void QRenderPrivate::setWindowSize(int w, int h)
{
    m_window = QSize(w, h);
}

bool QRenderPrivate::setScale(int windowWidth, int windowHeight)
{
    if(m_video.width() == windowWidth && m_video.height() == windowHeight)
        return false;

    float fX = 1.0;
    float fY = 1.0;

    switch (m_scaleType) {
    case QPlayMenu::play_size_video:
        scale(m_video.width(), m_video.height(), windowWidth, windowHeight, fX, fY);
        break;
    case QPlayMenu::play_size_window:
        break;
    case QPlayMenu::play_size_16_9:
    {
        auto minSize = scale(16, 9);
        scale(minSize.width(), minSize.height(), windowWidth, windowHeight, fX, fY);
    }break;
    case QPlayMenu::play_size_4_3:
    {
        auto minSize = scale(4, 3);
        scale(minSize.width(), minSize.height(), windowWidth, windowHeight, fX, fY);
    }break;
    }

    if(qFuzzyCompare(m_fScaleX, fX) && qFuzzyCompare(m_fScaleY, fY))
        return false;

    m_fScaleX = fX;
    m_fScaleY = fY;

    return true;
}

void QRenderPrivate::setScaleType(int type)
{
    if(type >= QPlayMenu::play_size_max)
        return;

    if(m_scaleType != type)
    {
        m_scaleType = type;
        switch (m_scaleType) {
        case QPlayMenu::play_size_video:
            break;
        case QPlayMenu::play_size_window:
            break;
        case QPlayMenu::play_size_16_9:
            break;
        case QPlayMenu::play_size_4_3:
            break;
        }


    }
}

void QRenderPrivate::removeFrame()
{
    if(m_frame)
    {
        m_frame.reset();
    }
}

QSize QRenderPrivate::scale(int w, int h)
{
    auto f0 = m_video.width() * 1.0f / w;
    auto f1 = m_video.height() * 1.0f / h;
    auto minScale = qMin(f0, f1);
    auto size = QSize(static_cast<int>(minScale * w), static_cast<int>(minScale * h));
    return size;
}

void QRenderPrivate::scale(int frameW, int frameH, int w, int h, float &x, float &y)
{
    auto f0 = frameW * 1.0f / frameH;
    auto f1 = w * 1.0f / h;
    if(f0 > f1)
        y = f1 / f0;
    else if(f0 < f1)
        x = f0 / f1;
}
