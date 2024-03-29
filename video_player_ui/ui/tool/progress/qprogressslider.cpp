#include "qprogressslider.h"
#include <QMouseEvent>
#include <QApplication>
#include <QToolTip>
#include <QTime>
#include <QLabel>
#include <QBoxLayout>
#include "framelesswidget/util.h"
#include "control/videocontrol.h"

QProgressSlider::QProgressSlider(Qt::Orientation orientation, QWidget* parent, QWidget* grandParent)
    : QSlider(orientation, parent)
    , m_nJumpStep(0)
    , m_nPreview(0)
    , m_nJumpCount(0)
    , m_bHandup(false)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    m_timeWidget = new QWidget(grandParent, Qt::FramelessWindowHint | Qt::ToolTip);
    m_time = new QLabel("00:00:00", m_timeWidget);
    m_timeWidget->hide();
    m_timeWidget->setObjectName("time_wd");
    m_time->setObjectName("label_time");
    auto layout = new QVBoxLayout(m_timeWidget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setMargin(0);
    layout->addWidget(m_time);
    CALC_WIDGET_SIZE(m_timeWidget, 65, 26);
    auto size = CALC_WIDGET_SIZE(nullptr, -20, 35);
    m_timePt.setX(size.width());
    m_timePt.setY(size.height());
    m_preview = new QLabel(grandParent);
    m_preview->setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    m_preview->setAlignment(Qt::AlignCenter);
    CALC_WIDGET_SIZE(m_preview, 200, 150);
#ifdef Q_OS_WIN
    auto margin = CALC_WIDGET_WIDTH(nullptr, 5);
    UTIL->setWindowEllispeFrame(m_preview, margin, margin);
#endif
    m_preview->hide();
    m_preview->setObjectName("label_preview");
    m_getPreview = new QTimer(this);
    m_getPreview->setInterval(30);
    m_getPreview->setSingleShot(true);
    connect(m_getPreview, &QTimer::timeout, this, [this]{ emit getPreview(m_nPreview); });

    m_jumpTimer = new QTimer(this);
    m_jumpTimer->setInterval(100);
    m_jumpTimer->setSingleShot(true);
    connect(m_jumpTimer, &QTimer::timeout, this, &QProgressSlider::onJumpTimer);
}

void QProgressSlider::setPos(int pos)
{
    if(m_bHandup)
        return;
    if(m_jumpTimer->isActive())
        return;
    setValue(pos);
    if(m_nJumpCount > 0)
    {
        QString s = QTime::fromMSecsSinceStartOfDay(pos).toString("HH:mm:ss");
        emit jumpStr(s);
        --m_nJumpCount;
    }
}

void QProgressSlider::onPreview(void *data, int width, int height)
{
    if(underMouse())
    {
        auto img = _video_frame_::toImage(data, width, height);
        auto pixmap = QPixmap::fromImage(img).scaled(m_preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_preview->setPixmap(pixmap);
        m_preview->show();
    }
}

void QProgressSlider::onJump(bool bFuture)
{
    if(maximum() > 100000000)
        return;

    if(!VIDEO_CONTROL->isPlaying() && !VIDEO_CONTROL->isPause())
        return;

    int step = (bFuture ? 10000 : -10000);
    if(m_nJumpCount <= 0)
    {
        emit jumpPos(step);
        m_nJumpCount += 2;
    }
    else
    {
        m_jumpTimer->stop();
        m_jumpTimer->start();
        m_nJumpStep = value() + step;
        setValue(m_nJumpStep);

        QString s = QTime::fromMSecsSinceStartOfDay(m_nJumpStep).toString("HH:mm:ss");
        emit jumpStr(s);
    }
}

void QProgressSlider::onJumpFailed()
{
    if(m_nJumpCount > 2)
        m_nJumpCount -= 2;
}

void QProgressSlider::onJumpTimer()
{
    emit gotoPos(m_nJumpStep);
    m_nJumpCount += 2;
    m_nJumpStep = 0;
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
    auto pro = pos.x() * 1.0f / width();
    int num = static_cast<int>(pro * maximum());
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
    auto pro = pos.x() * 1.0f / width();
    m_nPreview = static_cast<int>(pro * maximum());

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
