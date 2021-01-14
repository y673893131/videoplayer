/*
 *
 */
#pragma comment(lib, "d3d11.lib")

#include "qd3d11widget.h"

#include <QDebug>
#include <QEvent>
#include <QWheelEvent>
#include <QString>
#include <QApplication>
#include <QScreen>

QD3D11Widget::QD3D11Widget(QWidget * parent)
    : QWidget(parent)
    , m_hWnd(reinterpret_cast<HWND>(winId()))
    , m_width(width())
    , m_height(height())
    , m_pGraphics(Q_NULLPTR)
    , m_bDeviceInitialized(false)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
//    setAttribute(Qt::WA_TransparentForMouseEvents);
//    setAttribute(Qt::WA_OpaquePaintEvent);
}

QD3D11Widget::~QD3D11Widget() {}

void QD3D11Widget::setSize(int w, int h)
{
    m_width = w;
    m_height = h;
}

void QD3D11Widget::release()
{
    // Release the graphics object.
    if(m_pGraphics)
    {
        m_pGraphics->Shutdown();
        delete m_pGraphics;
        m_pGraphics = 0;
    }
}

void QD3D11Widget::showEvent(QShowEvent * event)
{
    if (!m_bDeviceInitialized)
    {
        m_bDeviceInitialized = initialize();
        emit deviceInitialized(m_bDeviceInitialized);
    }

    QWidget::showEvent(event);
}

void QD3D11Widget::onFrame()
{
//    if (m_bRenderActive) tick();
    render();
//    update();
}

void QD3D11Widget::render()
{
    // TODO: Present your scene here. For aesthetics reasons, only do it here if it's an
    // important component, otherwise do it in the MainWindow.
    // m_pCamera->Apply();

    emit rendered();
}

bool QD3D11Widget::initialize()
{
    // Create the graphics object.  This object will handle rendering all the graphics for this application.
    m_pGraphics = new GraphicsClass;
    if(!m_pGraphics)
    {
        return false;
    }

    // Initialize the graphics object.
    auto result = m_pGraphics->Initialize(m_width, m_height, m_hWnd);
    if(!result)
    {
        return false;
    }

    return true;
}

QPaintEngine* QD3D11Widget::paintEngine() const
{
    return Q_NULLPTR;
}

void QD3D11Widget::paintEvent(QPaintEvent * /*event*/)
{
    render();
}

void QD3D11Widget::resizeEvent(QResizeEvent * event)
{
    m_width = width();
    m_height = height();

    QWidget::resizeEvent(event);
}
