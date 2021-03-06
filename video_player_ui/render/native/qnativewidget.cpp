#include "qnativewidget.h"
#include "../../config/config.h"
#include <QDebug>

QNativeWidget::QNativeWidget(QWidget *parent)
    : QWidget(parent)
    , m_width(0)
    , m_height(0)
    , m_bDeviceInitialized(false)
    , m_bViewAdjust(true)
    , m_pFrame(nullptr)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);

    connect(this, &QNativeWidget::flush, this, &QNativeWidget::render);
    connect(Config::instance(), &Config::loadConfig, [this]
    {
        m_bViewAdjust = GET_CONFIG_DATA(Config::Data_Adjust).toBool();
        qDebug() << "m_bViewAdjust" << m_bViewAdjust;
    });
}

QNativeWidget::~QNativeWidget()
{
}

void QNativeWidget::onAppendFrame(void *frame)
{
    if(m_pFrame) delete m_pFrame;
    m_pFrame = reinterpret_cast<_video_frame_*>(frame);
    emit flush();
}

QPaintEngine* QNativeWidget::paintEngine() const
{
    return Q_NULLPTR;
}

void QNativeWidget::showEvent(QShowEvent * event)
{
    if (!m_bDeviceInitialized)
    {
        m_bDeviceInitialized = initialize();
        emit deviceInitialized(m_bDeviceInitialized);
    }

    QWidget::showEvent(event);
}

void QNativeWidget::paintEvent(QPaintEvent * /*event*/)
{
    render();
}

void QNativeWidget::resizeEvent(QResizeEvent * event)
{
    m_width = static_cast<unsigned int>(width());
    m_height = static_cast<unsigned int>(height());

    QWidget::resizeEvent(event);
}
