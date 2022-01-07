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
    auto fileList = new QPushButton(this);
    auto time = new QLabel(this);
    m_label[label_time] = time;
    time->setAlignment(Qt::AlignCenter);
    auto framRate = new QLabel(this);
    m_label[label_rate] = framRate;

    m_label[label_time]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_label[label_rate]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_button[button_stop] = stop;
    m_button[button_prev] = prev;
    m_button[button_play_or_continue] = play;
    m_button[button_play_or_continue]->setCheckable(true);
    m_button[button_next] = next;
    m_button[button_vol_mute] = volMute;
    m_button[button_file_list] = fileList;

    stop->setObjectName("btn_stop");
    prev->setObjectName("btn_prev");
    play->setObjectName("btn_play");
    next->setObjectName("btn_next");
    volMute->setObjectName("btn_volume");
    fileList->setObjectName("file_list");
    fileList->setToolTip(tr("play list"));
    time->setObjectName("label_frame_time");
    framRate->setObjectName("label_frame_rate");

    m_button[button_vol_mute]->setToolTip(tr("set mute"));

    auto btnSize = CALC_WIDGET_SIZE(nullptr, 50, 50);
    stop->setFixedSize(btnSize / 6 * 2);
    prev->setFixedSize(btnSize / 5 * 2.5);
    play->setFixedSize(btnSize / 5 * 2.5);
    next->setFixedSize(btnSize / 5 * 2.5);
    volMute->setFixedSize(btnSize / 2);
    CALC_WIDGET_WIDTH(m_label[label_time], 150);
    CALC_WIDGET_WIDTH(m_label[label_rate], 100);
    fileList->setFixedSize(btnSize / 5 * 3);

    volMute->setCheckable(true);

    m_button[button_vol_mute]->installEventFilter(this);
}

void QPlayControl::initLayout()
{
    auto layout = new QHBoxLayout(this);
    auto space = CALC_WIDGET_WIDTH(nullptr, 15);
    auto margin = CALC_WIDGET_WIDTH(nullptr, 10);
    layout->setSpacing(space);
    layout->setContentsMargins(margin, 0, margin, 0);

    layout->addWidget(m_label[label_time]);
    layout->addStretch();
    layout->addWidget(m_button[button_stop]);
    layout->addWidget(m_button[button_prev]);
    layout->addWidget(m_button[button_play_or_continue]);
    layout->addWidget(m_button[button_next]);
    layout->addWidget(m_button[button_vol_mute]);
    layout->addStretch();
    layout->addWidget(m_label[label_rate]);
    layout->addSpacing(10);
    layout->addWidget(m_button[button_file_list]);
}

void QPlayControl::initConnect()
{
    auto control = VIDEO_CONTROL;
    auto fileList = m_parent->findChild<QFileView*>();
    connect(control, &QVideoControl::total, this, &QPlayControl::onTotal);
    connect(control, &QVideoControl::frameRate, this, &QPlayControl::onRate);
    connect(control, &QVideoControl::end, this, &QPlayControl::onStop);
    connect(control, &QVideoControl::setPos, this, &QPlayControl::onUpdateCurrentTime);

    connect(m_button[button_play_or_continue], &QAbstractButton::released, this, &QPlayControl::onPlayOrPause);
    connect(m_button[button_stop], &QAbstractButton::clicked, fileList, &QFileView::onHandleStop);
    connect(m_button[button_stop], &QAbstractButton::clicked, control, &QVideoControl::onStoped);
    connect(m_button[button_vol_mute], &QAbstractButton::clicked, control, &QVideoControl::onSetMute);
    connect(m_button[button_vol_mute], &QAbstractButton::clicked, this, &QPlayControl::onMute);
    connect(m_button[button_file_list], &QAbstractButton::clicked, fileList, &QFileView::onAutoShow);
    connect(m_button[button_prev], &QAbstractButton::clicked, fileList, &QFileView::onPrev);
    connect(m_button[button_next], &QAbstractButton::clicked, fileList, &QFileView::onNext);

    connect(Config::instance(), &Config::loadConfig, this, &QPlayControl::onLoadConfig);

    connect(this, &QPlayControl::pause, control, &QVideoControl::onPause);
    connect(this, &QPlayControl::continuePlay, control, &QVideoControl::onContinue);

    connect(QInputFilter::instance(), &QInputFilter::space, this, &QPlayControl::onPlayOrPause);
}

void QPlayControl::onPlay()
{
    auto btn = m_button[button_play_or_continue];
    btn->setObjectName("btn_pause");
    btn->setStyleSheet(qApp->styleSheet());
}

void QPlayControl::onPause()
{
    auto btn = m_button[button_play_or_continue];
    btn->setObjectName("btn_play");
    btn->setStyleSheet(qApp->styleSheet());
}

void QPlayControl::onPlayOrPause()
{
    auto btn = m_button[button_play_or_continue];
    if(m_bPlaying)
    {
        auto bPause = btn->objectName() == "btn_play";
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
    m_label[label_time]->clear();
    m_label[label_rate]->clear();

    m_nTotal = 0;
    m_bPlaying = false;
    onPause();
}

void QPlayControl::onMute(bool bChecked)
{
    SET_CONFIG_DATA(bChecked, Config::Data_Mute);
    m_button[button_vol_mute]->setToolTip(bChecked ? tr("close mute") : tr("set mute"));
}

void QPlayControl::onRate(int frameCount)
{
    auto label = m_label[label_rate];
    label->setText(QString("%1: %2/s").arg(tr("video")).arg(frameCount));
}

void QPlayControl::onLoadConfig()
{
    auto mute = m_button[button_vol_mute];
    mute->setChecked(GET_CONFIG_DATA(Config::Data_Mute).toBool());
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

bool QPlayControl::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_button[button_vol_mute])
    {
        switch (event->type()) {
        case QEvent::Enter:
            emit showVolume(true, mapToGlobal(m_button[button_vol_mute]->pos()), m_button[button_vol_mute]->size());
            break;
        case QEvent::Leave:
            emit showVolume(false, mapToGlobal(m_button[button_vol_mute]->pos()), m_button[button_vol_mute]->size());
            break;
        }
    }

    return QToolBase::eventFilter(watched, event);
}
