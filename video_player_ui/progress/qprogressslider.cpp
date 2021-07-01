#include "qprogressslider.h"
#include <QMouseEvent>
#include <QApplication>
#include <QToolTip>
#include <QTime>
#include <QLabel>
#include <QBoxLayout>
#include <QDebug>
#include "framelesswidget/util.h"

QProgressSlider::QProgressSlider(Qt::Orientation orientation, QWidget* parent, QWidget* grandParent)
    : QSlider(orientation, parent)
    , m_nPreview(0)
    , m_bHandup(false)
{
    setMouseTracking(true);
    m_timeWidget = new QWidget(grandParent, Qt::FramelessWindowHint | Qt::ToolTip);
    m_time = new QLabel("00:00:00", m_timeWidget);
    m_timeWidget->hide();
    m_timeWidget->setObjectName("time_wd");
    m_time->setObjectName("label_time");
    auto layout = new QVBoxLayout(m_timeWidget);
    auto margin = CALC_WIDGET_WIDTH(nullptr, 5);
    layout->setAlignment(Qt::AlignCenter);
    layout->setMargin(0);
    layout->addWidget(m_time);
    CALC_WIDGET_SIZE(m_timeWidget, 65, 26);
    UTIL->setWindowEllispeFrame(m_timeWidget, margin / 2, margin / 2);
    auto size = CALC_WIDGET_SIZE(nullptr, -20, 35);
    m_timePt.setX(size.width());
    m_timePt.setY(size.height());

    m_preview = new QLabel(grandParent);
    m_preview->setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    CALC_WIDGET_SIZE(m_preview, 200, 150);
    UTIL->setWindowEllispeFrame(m_preview, margin, margin);
    m_preview->hide();
    m_preview->setObjectName("label_preview");

    m_getPreview = new QTimer(this);
    m_getPreview->setInterval(30);
    m_getPreview->setSingleShot(true);
    connect(m_getPreview, &QTimer::timeout, [this]{ emit getPreview(m_nPreview); });
}

void QProgressSlider::setPos(int pos)
{
    if(m_bHandup)
        return;
    setValue(pos);
}

void QProgressSlider::onPreview(void *data, int width, int height)
{
    if(underMouse())
    {
//        unsigned int w = static_cast<unsigned int>(width);
//        unsigned int h = static_cast<unsigned int>(height);
//        auto _data = reinterpret_cast<unsigned char*>(data);
//        char* rgb = new char[w * h * 3];
//        auto _rgb = reinterpret_cast<unsigned char*>(rgb);
//        auto _rgb0 = reinterpret_cast<uchar*>(rgb);
//        memset(rgb, 0x00, w * h * 3);
//        yuv420p_to_rgb24(_data, _rgb, width, height);
//        QImage img(_rgb0, width, height, QImage::Format_RGB888);
//        auto pixmap = QPixmap::fromImage(img).scaled(m_preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
//        m_preview->setPixmap(pixmap);
//        delete[] rgb;

        auto _data = reinterpret_cast<unsigned char*>(data);
        QImage img(_data, width, height, QImage::Format_RGB888);
        auto pixmap = QPixmap::fromImage(img).scaled(m_preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_preview->setPixmap(pixmap);
        m_preview->show();
    }
}

void QProgressSlider::mousePressEvent(QMouseEvent *event)
{
    QSlider::mousePressEvent(event);
    m_bHandup = true;
}

void QProgressSlider::mouseReleaseEvent(QMouseEvent *event)
{
    QSlider::mouseReleaseEvent(event);
    auto pos = event->pos();
    auto pro = pos.x() * 1.0 / width();
    int num = pro * maximum();
    setValue(num);
    emit gotoPos(num);
    m_bHandup = false;
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
    m_getPreview->stop();
    m_bHandup = false;
    m_timeWidget->hide();
}

void QProgressSlider::mouseMoveEvent(QMouseEvent *event)
{
    QSlider::mouseMoveEvent(event);

    auto pos = event->pos();
    auto pro = pos.x() * 1.0 / width();
    m_nPreview = pro * maximum();

    pos.setY(0);
    pos = mapToGlobal(pos);

    QString s = QTime::fromMSecsSinceStartOfDay(m_nPreview).toString("HH:mm:ss");
    m_time->setText(s);
    m_timeWidget->move(pos + m_timePt);
    m_timeWidget->show();

    m_preview->move(pos.x() - m_preview->width() / 2, pos.y() - m_preview->height());
    m_getPreview->stop();
    m_getPreview->start();
}
