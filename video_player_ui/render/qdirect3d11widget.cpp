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
    auto result = m_pGraphics->Initialize(m_width, m_height, reinterpret_cast<HWND>(winId()));
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
        m_pGraphics = 0;
    }
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
