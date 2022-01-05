#include "qplaycontrol.h"
#include <QApplication>
#include <QPushButton>
#include <QButtonGroup>
#include <QStyle>
#include <QSlider>
#include <QLabel>
#include <QBoxLayout>
#include <QTime>
#include "framelesswidget/util.h"
#include "control/videocontrol.h"
#include "ui/qtoolwidgets.h"
#include "config/config.h"
#include "ui/tool/fileview/qfileview.h"
#include "ui/tool/volume/qcolumeslider.h"
#include "ui/tool/output/qoutputwidget.h"
#include "filter/qinputfilter.h"

QPlayControl::QPlayControl(QWidget* parent)
    :QToolBase(parent)
    ,m_bPlaying(false)
    ,m_nTotal(0)
{
    setObjectName("wd_toolbar");
    initUi();
    initLayout();
}

void QPlayControl::initUi()
{
    auto stop = new QPushButton(this);
    auto prev = new QPushButton(this);
    auto play = new QPushButton(this);
    auto next = new QPushButton(this);
    auto volMute = new QPushButton(this);
    m_vol = new QColumeSlider(Qt::Orientation::Horizontal, this);
    auto fileList = new QPushButton(this);
    auto time = new QLabel(this);
    m_label[label_time] = time;
    time->setAlignment(Qt::AlignCenter);
    auto framRate = new QLabel(this);
    m_label[label_rate] = framRate;

    m_button[button_stop] = stop;
    m_button[button_prev] = prev;
    m_button[button_play_or_continue] = play;
    m_button[button_play_or_continue]->setCheckable(true);
    m_button[button_next] = next;
    m_button[button_vol_mute] = volMute;
    m_button[button_file_list] = fileList;

//    setObjectName("wd_toolbar");
    stop->setObjectName("btn_stop");
    prev->setObjectName("btn_prev");
    play->setObjectName("btn_pause");
    next->setObjectName("btn_next");
    volMute->setObjectName("btn_volume");
    volMute->setIcon(/*parent->*/style()->standardIcon(QStyle::SP_MediaVolume));
    fileList->setObjectName("file_list");
    fileList->setToolTip(tr("play list"));
    time->setObjectName("label_frame_time");
    framRate->setObjectName("label_frame_rate");

    auto btnSize = CALC_WIDGET_SIZE(nullptr, 50, 50);
    stop->setFixedSize(btnSize / 5 * 2);
    prev->setFixedSize(btnSize);
    play->setFixedSize(btnSize);
    next->setFixedSize(btnSize);
    volMute->setFixedSize(btnSize / 2);
    CALC_WIDGET_WIDTH(m_vol, 100);
    CALC_WIDGET_WIDTH(m_label[label_time], 150);
    CALC_WIDGET_WIDTH(m_label[label_rate], 100);
    fileList->setFixedSize(btnSize / 5 * 3);

    volMute->setCheckable(true);

    m_timerSetVol = new QTimer(this);
    m_timerSetVol->setInterval(500);
    m_timerSetVol->setSingleShot(true);
}

void QPlayControl::initLayout()
{
    auto layout = new QHBoxLayout(this);
    auto space = CALC_WIDGET_WIDTH(nullptr, 10);
    auto margin = CALC_WIDGET_WIDTH(nullptr, 15);
    layout->setSpacing(space);
    layout->setMargin(margin);

    layout->addWidget(m_label[label_time]);
    layout->addStretch();
    layout->addWidget(m_button[button_stop]);
    layout->addWidget(m_button[button_prev]);
    layout->addWidget(m_button[button_play_or_continue]);
    layout->addWidget(m_button[button_next]);
    layout->addWidget(m_button[button_vol_mute]);
    layout->addWidget(m_vol);
    layout->addStretch();
    layout->addWidget(m_label[label_rate]);
    layout->addSpacing(10);
    layout->addWidget(m_button[button_file_list]);
}

void QPlayControl::initConnect()
{
    auto control = VIDEO_CONTROL;
    auto fileList = m_parent->findChild<QFileView*>();
    auto output = m_parent->findChild<QOutputWidget*>();
    auto toolWidget = qobject_cast<QToolWidgets*>(m_parent);
    connect(control, &QVideoControl::total, this, &QPlayControl::onTotal);
    connect(control, &QVideoControl::frameRate, this, &QPlayControl::onRate);
    connect(control, &QVideoControl::end, this, &QPlayControl::onStop);
    connect(control, &QVideoControl::setPos, this, &QPlayControl::onUpdateCurrentTime);

    connect(m_button[button_play_or_continue], &QAbstractButton::released, this, &QPlayControl::onPlayOrPause);
    connect(m_button[button_stop], &QAbstractButton::clicked, control, &QVideoControl::onStoped);
    connect(m_button[button_vol_mute], &QAbstractButton::clicked, control, &QVideoControl::onSetMute);
    connect(m_button[button_vol_mute], &QAbstractButton::clicked, this, &QPlayControl::onMute);
    connect(m_button[button_file_list], &QAbstractButton::clicked, fileList, &QFileView::onAutoShow);
    connect(m_button[button_prev], &QAbstractButton::clicked, fileList, &QFileView::onPrev);
    connect(m_button[button_next], &QAbstractButton::clicked, fileList, &QFileView::onNext);

    connect(Config::instance(), &Config::loadConfig, this, &QPlayControl::onLoadConfig);

    connect(this, &QPlayControl::pause, control, &QVideoControl::onPause);
    connect(this, &QPlayControl::continuePlay, control, &QVideoControl::onContinue);
    connect(m_vol, &QSlider::valueChanged, control, &QVideoControl::onSetVol);
    connect(m_vol, &QSlider::valueChanged, this, &QPlayControl::onVolChanged);
    connect(m_vol, &QColumeSlider::jumpStr, output, &QOutputWidget::onInfo);

    connect(m_timerSetVol, &QTimer::timeout, this, &QPlayControl::onSetVol);
    connect(QInputFilter::instance(), &QInputFilter::volumeJump, m_vol, &QColumeSlider::onJump);
    connect(QInputFilter::instance(), &QInputFilter::space, this, &QPlayControl::onPlayOrPause);
}

void QPlayControl::onPlay()
{
    auto btn = m_button[button_play_or_continue];
    btn->setObjectName("btn_play");
    btn->setStyleSheet(qApp->styleSheet());
}

void QPlayControl::onPause()
{
    auto btn = m_button[button_play_or_continue];
    btn->setObjectName("btn_pause");
    btn->setStyleSheet(qApp->styleSheet());
}

void QPlayControl::onPlayOrPause()
{
    auto btn = m_button[button_play_or_continue];
    if(m_bPlaying)
    {
        auto bPause = btn->objectName() == "btn_pause";
        if(bPause)
        {
            emit continuePlay();
            onPlay();
        }
        else
        {
            emit pause();
            onPause();
        }

        return;
    }

    emit loadOrPlay();
}

void QPlayControl::onStop()
{
    auto label = m_label[label_rate];
    label->clear();

    m_nTotal = 0;
    m_bPlaying = false;
    onPause();
}

void QPlayControl::onMute(bool bChecked)
{
    auto btn = m_button[button_vol_mute];
    if(!bChecked)
        btn->setIcon(btn->style()->standardIcon(QStyle::SP_MediaVolume));
    else
        btn->setIcon(btn->style()->standardIcon(QStyle::SP_MediaVolumeMuted));
    SET_CONFIG_DATA(bChecked, Config::Data_Mute);
}

void QPlayControl::onRate(int frameCount)
{
    auto label = m_label[label_rate];
    label->setText(QString("%1: %2/s").arg(tr("video")).arg(frameCount));
}

void QPlayControl::onLoadConfig()
{
    m_vol->setValue(GET_CONFIG_DATA(Config::Data_Vol).toInt());
    auto mute = m_button[button_vol_mute];
    mute->setChecked(GET_CONFIG_DATA(Config::Data_Mute).toBool());
}

void QPlayControl::onVolChanged()
{
    m_timerSetVol->stop();
    m_timerSetVol->start();
}

void QPlayControl::onSetVol()
{
    SET_CONFIG_DATA(m_vol->value(), Config::Data_Vol);
}

void QPlayControl::onTotal(int nTotal)
{
    m_nTotal = nTotal;
    m_bPlaying = true;
    m_button[button_play_or_continue]->setObjectName("btn_pause");
    onUpdateCurrentTime(0);
    onPlay();
}

void QPlayControl::onUpdateCurrentTime(int nSeconds)
{
    auto time = m_label[label_time];
    if(m_nTotal > 100000000)
    {
        time->clear();
    }
    else
    {
        time->setText(QString("%1 / %2").arg(QTime::fromMSecsSinceStartOfDay(nSeconds).toString("HH:mm:ss"))
                      .arg(QTime::fromMSecsSinceStartOfDay(m_nTotal).toString("HH:mm:ss")));
    }
}
