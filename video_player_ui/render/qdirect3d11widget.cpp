#ifdef WIN32
#include "qdirect3d11widget.h"
#include <QDebug>
#include <QTime>
#include "config/config.h"
#include "Log/Log.h"

QDirect3D11Widget::QDirect3D11Widget(QWidget * parent)
    : QNativeWidget(parent)
    , m_bStoped(true)
{
}

QDirect3D11Widget::~QDirect3D11Widget()
{
    release();
}

bool QDirect3D11Widget::initialize()
{
    // Create the graphics object.  This object will handle rendering all the graphics for this application.
    m_pGraphics = new GraphicsClass;
    if(!m_pGraphics)
    {
        return false;
    }

    // Initialize the graphics object.
    auto result = m_pGraphics->Initialize(m_width, m_height, reinterpret_cast<HWND>(winId()), m_fScaleX, m_fScaleY);
    if(!result)
    {
        return false;
    }

    return true;
}

void QDirect3D11Widget::release()
{
    // Release the graphics object.
    if(m_pGraphics)
    {
        m_pGraphics->Shutdown();
        delete m_pGraphics;
        m_pGraphics = nullptr;
    }
}

void QDirect3D11Widget::render()
{
    if(!m_bStoped)
    {
        if(m_pFrame)
            m_pGraphics->Frame(m_pFrame);
        else if(m_freq)
        {
            if(m_freq)
                m_pGraphics->Freq(m_freq, m_freqCount);
            else
                m_pGraphics->Freq(Q_NULLPTR, m_freqCount);
        }
    }
    else
        m_pGraphics->Frame(Q_NULLPTR);
}

void QDirect3D11Widget::onViewAdjust(bool bViewAdjust)
{
    if(m_bViewAdjust != bViewAdjust)
    {
        m_bViewAdjust = bViewAdjust;
        if(m_pGraphics)
        {
            if(m_bViewAdjust)
                m_pGraphics->ResetViewport(m_fScaleX, m_fScaleY);
            else
                m_pGraphics->ResetViewport(1.0f, 1.0f);
        }
    }
}

void QDirect3D11Widget::onVideoSizeChanged(int width, int height)
{
    float f0 = width * 1.0f / height;
    float f1 = this->width() * 1.0f / this->height();
    float fX = 1.0;
    float fY = 1.0;
    if(f0 > f1)
        fY = f1 / f0;
    else if(f0 < f1)
        fX = f0 / f1;

    if(m_fScaleX == fX && m_fScaleY == fY)
        return;
    m_fScaleX = fX;
    m_fScaleY = fY;

    if(m_bViewAdjust)
    {
        m_pGraphics->ResetViewport(m_fScaleX, m_fScaleY);
    }
    qDebug() << __FUNCTION__ << m_fScaleX << m_fScaleY;
}

void QDirect3D11Widget::onAppendFreq(float *data, unsigned int size)
{

    if(!m_pFrame)
    {
        removeFreq();
        m_freq = data;
        m_freqCount = size;
        update();
    }
    else
    {
        delete[] data;
    }
}

void QDirect3D11Widget::onStart()
{
    m_bStoped = false;
}

void QDirect3D11Widget::onStop()
{
    m_bStoped = true;
    removeFrame();
    removeFreq();
    update();
}

#endif
