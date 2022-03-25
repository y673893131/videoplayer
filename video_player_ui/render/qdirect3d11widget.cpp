#ifdef WIN32
#include "qdirect3d11widget.h"
#include <QDebug>
#include <QTime>
#include "config/config.h"
#include "Log/Log.h"

#include <D3D11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <D3DX10math.h>
#include "dx11/inputclass.h"
#include "dx11/graphicsclass.h"

#include "qrenderprivate.h"
#include "native/qnativewidget_p.h"

class QDirect3D11WidgetPrivate : public QNativeWidgetPrivate
{
    VP_DECLARE_PUBLIC(QDirect3D11Widget)
public:
    inline QDirect3D11WidgetPrivate(QDirect3D11Widget* parent)
        : QNativeWidgetPrivate(parent)
        , m_bStoped(true)
    {
    }

    ~QDirect3D11WidgetPrivate()
    {
        if(m_pGraphics)
        {
            m_pGraphics->Shutdown();
            delete m_pGraphics;
            m_pGraphics = nullptr;
        }
    }

    GraphicsClass* m_pGraphics;
    bool m_bStoped;
};

QDirect3D11Widget::QDirect3D11Widget(QWidget * parent)
    : QNativeWidget(new QDirect3D11WidgetPrivate(this), parent)
{
}

QDirect3D11Widget::~QDirect3D11Widget()
{
}

bool QDirect3D11Widget::init(const QSize& size, float x, float y)
{
    VP_D(QDirect3D11Widget);

    // Create the graphics object.  This object will handle rendering all the graphics for this application.
    d->m_pGraphics = new GraphicsClass;
    if(!d->m_pGraphics)
    {
        return false;
    }

    // Initialize the graphics object.
    auto result = d->m_pGraphics->Initialize(
                static_cast<unsigned int>(size.width()),
                static_cast<unsigned int>(size.height()),
                reinterpret_cast<HWND>(winId()), x, y);
    if(!result)
    {
        return false;
    }

    return true;
}

void QDirect3D11Widget::render(_VideoFramePtr frame, float* freq, unsigned int freqCount)
{
    VP_D(QDirect3D11Widget);
    if(!d->m_bStoped)
    {
        if(frame)
            d->m_pGraphics->Frame(frame);
        else if(freq)
        {
            if(freq)
                d->m_pGraphics->Freq(freq, freqCount);
            else
                d->m_pGraphics->Freq(Q_NULLPTR, freqCount);
        }
    }
    else
        d->m_pGraphics->Frame(Q_NULLPTR);
}

void QDirect3D11Widget::resetView(float x, float y)
{
    VP_D(QDirect3D11Widget);
    if(d->m_pGraphics)
    {
        qDebug() << "scale:" << x << y;
        d->m_pGraphics->ResetViewport(x, y);
    }
}

void QDirect3D11Widget::onStart()
{
    VP_D(QDirect3D11Widget);
    d->m_bStoped = false;
}

void QDirect3D11Widget::onStop()
{
    VP_D(QDirect3D11Widget);
    d->m_bStoped = true;
    removeFrame();
    update();
}

#endif
