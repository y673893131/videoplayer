#include "qnativewidget.h"
#include "../../config/config.h"
#include <QDebug>
#include "../qrenderprivate.h"
#include "qnativewidget_p.h"

QNativeWidget::QNativeWidget(QWidget *parent)
    : QWidget(parent)
    , VP_INIT(new QNativeWidgetPrivate(this))
{
    setup();
}

QNativeWidget::QNativeWidget(QNativeWidgetPrivate *pri, QWidget *parent)
    : QWidget(parent)
    , VP_INIT(pri)
{
    setup();
}

QNativeWidget::~QNativeWidget()
{
}

void QNativeWidget::setup()
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
//    setAutoFillBackground(true);
//    setFocusPolicy(Qt::StrongFocus);
//    setAttribute(Qt::WA_NativeWindow);
//    setAttribute(Qt::WA_PaintOnScreen);
//    setAttribute(Qt::WA_NoSystemBackground);

    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_PaintOnScreen, true);

    auto func = [this]
    {
        VP_D(QNativeWidget);
        d->initConfig();
        onViewAdjust(true);
    };

    connect(Config::instance(), &Config::loadConfig, this, func);
    connect(Config::instance(), &Config::setConfig, this, func);
}


void QNativeWidget::onAppendFrame(_VideoFramePtr frame)
{
    VP_D(QNativeWidget);
    d->m_frame = frame;
    update();
}

void QNativeWidget::onViewAdjust(bool bViewAdjust)
{
    VP_D(QNativeWidget);
    if(d->setScale(width(), height()) || bViewAdjust)
        resetView(d->m_fScaleX, d->m_fScaleY);
}

void QNativeWidget::onVideoSizeChanged(int w, int h)
{
    VP_D(QNativeWidget);
    d->setVideoSize(w, h);
    onViewAdjust(false);
}

void QNativeWidget::onAppendFreq(float *data, unsigned int size)
{
    VP_D(QNativeWidget);
    if(!d->m_frame)
    {
        d->m_freq = data;
        d->m_freqCount = size;
        update();
    }
}

bool QNativeWidget::initialize()
{
    VP_D(QNativeWidget);
    return init(d->m_window, d->m_fScaleX, d->m_fScaleY);
}

void QNativeWidget::removeFrame()
{
    VP_D(QNativeWidget);
    d->removeFrame();
}

void QNativeWidget::render()
{
    VP_D(QNativeWidget);
    render(d->m_frame, d->m_freq, d->m_freqCount);
}

QPaintEngine* QNativeWidget::paintEngine() const
{
    return Q_NULLPTR;
}

void QNativeWidget::showEvent(QShowEvent * event)
{
    VP_D(QNativeWidget);
    if (!d->m_bDeviceInitialized)
    {
        d->m_bDeviceInitialized = initialize();
        emit deviceInitialized(d->m_bDeviceInitialized);
    }

    QWidget::showEvent(event);
}

void QNativeWidget::paintEvent(QPaintEvent * /*event*/)
{
    render();
}

void QNativeWidget::resizeEvent(QResizeEvent * event)
{
    VP_D(QNativeWidget);
    d->setWindowSize(width(), height());
    QWidget::resizeEvent(event);
}
