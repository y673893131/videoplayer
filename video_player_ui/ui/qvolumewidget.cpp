#include "qvolumewidget.h"
#include "ui/tool/volume/qcolumeslider.h"
#include "ui/tool/output/qoutputwidget.h"
#include "ui/tool/play_control/qplaycontrol.h"
#include "filter/qinputfilter.h"
#include "control/videocontrol.h"
#include "config/config.h"
#include <QBoxLayout>

QVolumeWidget::QVolumeWidget(QWidget *parent)
    :QFrameLessWidget(parent), m_parent(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowModal, false);
#ifdef Q_OS_WIN
    setResizeable(false);
#endif
    setDragSelf(false);
    setObjectName("volume_widget");
    m_volume = new QColumeSlider(Qt::Orientation::Vertical, this);

    m_timerSetVol = new QTimer(this);
    m_timerSetVol->setInterval(500);
    m_timerSetVol->setSingleShot(true);

    m_timerHide = new QTimer(this);
    m_timerHide->setInterval(200);

    auto layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_volume);

    CALC_WIDGET_SIZE(this, 30, 100);
}

void QVolumeWidget::initConnect()
{
    auto control = VIDEO_CONTROL;
    auto playControl = m_parent->findChild<QPlayControl*>();
    auto output = m_parent->findChild<QOutputWidget*>();
    connect(m_volume, &QSlider::valueChanged, control, &QVideoControl::onSetVol);
    connect(m_volume, &QSlider::valueChanged, this, &QVolumeWidget::onVolChanged);
    connect(m_volume, &QColumeSlider::jumpStr, output, &QOutputWidget::onInfo);
    connect(QInputFilter::instance(), &QInputFilter::volumeJump, m_volume, &QColumeSlider::onJump);
    connect(m_timerSetVol, &QTimer::timeout, this, &QVolumeWidget::onSetVol);
    connect(m_timerHide, &QTimer::timeout, this, &QVolumeWidget::onTimerHide);
    connect(playControl, &QPlayControl::showVolume, this, &QVolumeWidget::onShow);

    connect(Config::instance(), &Config::loadConfig, this, &QVolumeWidget::onLoadConfig);
}

void QVolumeWidget::onLoadConfig()
{
    m_volume->setValue(GET_CONFIG_DATA(Config::Data_Vol).toInt());
}

void QVolumeWidget::onVolChanged()
{
    m_timerSetVol->stop();
    m_timerSetVol->start();
}

void QVolumeWidget::onSetVol()
{
    SET_CONFIG_DATA(m_volume->value(), Config::Data_Vol);
}

void QVolumeWidget::onShow(bool bVisable, const QPoint &pt, const QSize& size)
{
    auto pos = pt;
    pos.setX(pos.x() + (size.width() - width()) / 2);
    pos.setY(pos.y() - height());
    move(pos);
    m_timerHide->stop();
    if(bVisable)
        setVisible(bVisable);
    else
        m_timerHide->start();
}

void QVolumeWidget::onTimerHide()
{
    if(!underMouse())
    {
        setVisible(false);
        m_timerHide->stop();
    }
}
