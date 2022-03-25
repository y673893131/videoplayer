#include "qvolumewidget.h"
#include "ui/tool/volume/qcolumeslider.h"
#include "ui/tool/output/qoutputwidget.h"
#include "ui/tool/play_control/qplaycontrol.h"
#include "filter/qinputfilter.h"
#include "control/videocontrol.h"
#include "config/config.h"
#include <QBoxLayout>

#include "framelesswidget/nativeevent_p.h"

class QVolumeWidgetPrivate : public CNativeEvent_p
{
    VP_DECLARE_PUBLIC(QVolumeWidget)
    inline QVolumeWidgetPrivate(QVolumeWidget* parent, QWidget* grand)
        : CNativeEvent_p(parent)
        , m_parent(grand)
    {
        setResizeable(false);
    }

    QWidget* m_parent;
    QColumeSlider* m_volume;
    QTimer* m_timerSetVol;
    QTimer* m_timerHide;
};

QVolumeWidget::QVolumeWidget(QWidget *parent)
    :QFrameLessWidget(new QVolumeWidgetPrivate(this, parent), parent)
{
    VP_D(QVolumeWidget);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowModal, false);
    setDragSelf(false);
    setObjectName("volume_widget");
    d->m_volume = new QColumeSlider(Qt::Orientation::Vertical, this);

    d->m_timerSetVol = new QTimer(this);
    d->m_timerSetVol->setInterval(500);
    d->m_timerSetVol->setSingleShot(true);

    d->m_timerHide = new QTimer(this);
    d->m_timerHide->setInterval(200);

    auto layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(d->m_volume);

    CALC_WIDGET_SIZE(this, 30, 100);
}

void QVolumeWidget::initConnect()
{
    VP_D(QVolumeWidget);
    auto control = VIDEO_CONTROL;
    auto playControl = d->m_parent->findChild<QPlayControl*>();
    auto output = d->m_parent->findChild<QOutputWidget*>();
    connect(d->m_volume, &QSlider::valueChanged, control, &QVideoControl::onSetVol);
    connect(d->m_volume, &QSlider::valueChanged, this, &QVolumeWidget::onVolChanged);
    connect(d->m_volume, &QColumeSlider::jumpStr, output, &QOutputWidget::onInfo);
    connect(QInputFilter::instance(), &QInputFilter::volumeJump, d->m_volume, &QColumeSlider::onJump);
    connect(d->m_timerSetVol, &QTimer::timeout, this, &QVolumeWidget::onSetVol);
    connect(d->m_timerHide, &QTimer::timeout, this, &QVolumeWidget::onTimerHide);
    connect(playControl, &QPlayControl::showVolume, this, &QVolumeWidget::onShow);

    connect(Config::instance(), &Config::loadConfig, this, &QVolumeWidget::onLoadConfig);
}

void QVolumeWidget::onLoadConfig()
{
    VP_D(QVolumeWidget);
    d->m_volume->setValue(GET_CONFIG_DATA(Config::Data_Vol).toInt());
}

void QVolumeWidget::onVolChanged()
{
    VP_D(QVolumeWidget);
    d->m_timerSetVol->stop();
    d->m_timerSetVol->start();
}

void QVolumeWidget::onSetVol()
{
    VP_D(QVolumeWidget);
    SET_CONFIG_DATA(d->m_volume->value(), Config::Data_Vol);
}

void QVolumeWidget::onShow(bool bVisable, const QPoint &pt, const QSize& size)
{
    VP_D(QVolumeWidget);
    auto pos = pt;
    pos.setX(pos.x() + (size.width() - width()) / 2);
    pos.setY(pos.y() - height());
    move(pos);
    d->m_timerHide->stop();
    if(bVisable)
        setVisible(bVisable);
    else
        d->m_timerHide->start();
}

void QVolumeWidget::onTimerHide()
{
    VP_D(QVolumeWidget);
    if(!underMouse())
    {
        setVisible(false);
        d->m_timerHide->stop();
    }
}
