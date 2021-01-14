#include "qdirect3d11widget.h"
#include <QDebug>
#include <QTime>
#include "config.h"
#include "Log/Log.h"

QDirect3D11Widget::QDirect3D11Widget(QWidget * parent)
    : QD3D11Widget(parent)
    , m_pFrame(Q_NULLPTR)
    , m_bViewAdjust(true)
    , m_bStoped(true)
{
    connect(this, &QDirect3D11Widget::flush, this, &QDirect3D11Widget::onFrame);
    connect(Config::instance(), &Config::loadConfig, [this]
    {
        m_bViewAdjust = GET_CONFIG_DATA(Config::Data_Adjust).toBool();
        qDebug() << "m_bViewAdjust" << m_bViewAdjust;
    });
}

QDirect3D11Widget::~QDirect3D11Widget()
{
    release();
}

void QDirect3D11Widget::render()
{
    if(!m_bStoped)
        m_pGraphics->Frame(m_pFrame);
    else
        m_pGraphics->Frame(Q_NULLPTR);
}

void QDirect3D11Widget::onViewAdjust(bool)
{

}

void QDirect3D11Widget::onAppendFrame(void *frame)
{
    if(m_pFrame) delete m_pFrame;
    m_pFrame = reinterpret_cast<_video_frame_*>(frame);
    emit flush();
}

void QDirect3D11Widget::onVideoSizeChanged(int width, int height)
{
//    m_width = width;
//    m_height = height;
    float f0 = width * 1.0 / height;
    float f1 = this->width() * 1.0 / this->height();
    float fX = 1.0;
    float fY = 1.0;
    if(f0 > f1)
        fY = f1 / f0;
    else if(f0 < f1)
        fX = f0 / f1;

    static float fScaleX = 1.0;
    static float fScaleY = 1.0;
    if(fScaleX == fX && fScaleY == fY)
        return;
    fScaleX = fX;
    fScaleY = fY;

    qDebug() << __FUNCTION__ << fScaleX << fScaleY;
}

void QDirect3D11Widget::onStart()
{
    m_bStoped = false;
}

void QDirect3D11Widget::onStop()
{
    m_bStoped = true;
    emit flush();
}
