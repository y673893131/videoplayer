#include "qprogressslider.h"
#include <QMouseEvent>
#include <QApplication>
#include <QToolTip>
#include <QTime>
#include <QLabel>
#include <QDebug>
#include "framelesswidget/util.h"
#include "qlabelvideowidget.h"

QProgressSlider::QProgressSlider(Qt::Orientation orientation, QWidget* parent, QWidget* grandParent)
    : QSlider(orientation, parent)
    , m_nPreview(0)
{
    setMouseTracking(true);
    m_preview = new QLabel(grandParent);
    m_preview->setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    CALC_WIDGET_SIZE(m_preview, 200.0f / 1920, 150.0f / 1080);
    UTIL->setWindowEllispeFrame(m_preview, 5, 5);
    m_preview->hide();
    m_preview->setObjectName("label_preview");

    m_getPreview = new QTimer(this);
    m_getPreview->setInterval(30);
    m_getPreview->setSingleShot(true);
    connect(m_getPreview, &QTimer::timeout, [this]{ emit getPreview(m_nPreview); });

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    m_timer->setSingleShot(true);
//    connect(m_timer, &QTimer::timeout, this, [this]{ m_preview->hide(); });
}

void QProgressSlider::onPreview(void *data, int width, int height)
{
    if(underMouse())
    {
        unsigned int w = static_cast<unsigned int>(width);
        unsigned int h = static_cast<unsigned int>(height);
        auto _data = reinterpret_cast<unsigned char*>(data);
        char* rgb = new char[w * h * 3];
        auto _rgb = reinterpret_cast<unsigned char*>(rgb);
        auto _rgb0 = reinterpret_cast<uchar*>(rgb);
        memset(rgb, 0x00, w * h * 3);
        yuv420p_to_rgb24(_data, _rgb, width, height);
        QImage img(_rgb0, width, height, QImage::Format_RGB888);
        auto pixmap = QPixmap::fromImage(img).scaled(m_preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_preview->setPixmap(pixmap);
        delete[] rgb;
        m_preview->show();
        m_timer->start();
    }
}

void QProgressSlider::mousePressEvent(QMouseEvent *event)
{
    QSlider::mousePressEvent(event);
    auto pos = event->pos();
    auto pro = pos.x() * 1.0 / width();
    int num = pro * maximum();
    setValue(num);
    emit sliderMoved(num);
}

void QProgressSlider::enterEvent(QEvent *event)
{
    setCursor(Qt::PointingHandCursor);
    QSlider::enterEvent(event);
}

void QProgressSlider::leaveEvent(QEvent *event)
{
    QSlider::leaveEvent(event);
    m_preview->hide();
    m_preview->setPixmap(QPixmap());
    m_timer->stop();
    m_getPreview->stop();
}

void QProgressSlider::mouseMoveEvent(QMouseEvent *event)
{
    QSlider::mouseMoveEvent(event);

    auto pos = event->pos();
    auto pro = pos.x() * 1.0 / width();
    m_nPreview = pro * maximum();

    QString s = QTime::fromMSecsSinceStartOfDay(m_nPreview).toString("HH:mm:ss");
    QToolTip::showText(QCursor::pos() + QPoint(1, 5), s, this);

//    m_preview->show();
    pos.setY(0);
    pos = mapToGlobal(pos);
    m_preview->move(pos.x() - m_preview->width() / 2, pos.y() - m_preview->height());
    m_getPreview->stop();
    m_getPreview->start();
}
