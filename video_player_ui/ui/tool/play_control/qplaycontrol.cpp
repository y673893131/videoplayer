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
#include "ui/tool/menu/qplaymenu.h"
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
    m_label[label_time] =  new QLabel(this);
    m_label[label_time]->setAlignment(Qt::AlignCenter);
    m_label[label_rate] = new QLabel(this);

    m_label[label_time]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_label[label_rate]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    for(int n = 0; n < button_max; ++n)
    {
        m_button[n] = new QPushButton(this);
    }

    m_button[button_play_or_continue]->setCheckable(true);
    m_button[button_vol_mute]->setCheckable(true);

    m_button[button_stop]->setObjectName("btn_stop");
    m_button[button_prev]->setObjectName("btn_prev");
    m_button[button_play_or_continue]->setObjectName("btn_play");
    m_button[button_next]->setObjectName("btn_next");
    m_button[button_vol_mute]->setObjectName("btn_volume");
    m_button[button_mode]->setObjectName("btn_mode");
    m_button[button_mode]->setProperty("mode", QString::number(QPlayMenu::play_mode_loop));
    m_button[button_file_list]->setObjectName("file_list");
    m_label[label_time]->setObjectName("label_frame_time");
    m_label[label_rate]->setObjectName("label_frame_rate");

    m_button[button_stop]->setToolTip(tr("stop play"));
    m_button[button_prev]->setToolTip(tr("prev play"));
    m_button[button_next]->setToolTip(tr("next play"));

    m_button[button_file_list]->setToolTip(tr("play list"));
    m_button[button_vol_mute]->setToolTip(tr("set mute"));

    auto btnSize = CALC_WIDGET_SIZE(nullptr, 50, 50);
//    m_button[button_stop]->setFixedSize(btnSize / 6 * 2);
    m_button[button_stop]->setFixedSize(btnSize / 5 * 2.5);
    m_button[button_prev]->setFixedSize(btnSize / 5 * 2.5);
    m_button[button_play_or_continue]->setFixedSize(btnSize / 5 * 2.5);
    m_button[button_next]->setFixedSize(btnSize / 5 * 2.5);
    m_button[button_vol_mute]->setFixedSize(btnSize / 2);
    m_button[button_mode]->setFixedSize(btnSize / 2);
    CALC_WIDGET_WIDTH(m_label[label_time], 150);
    CALC_WIDGET_WIDTH(m_label[label_rate], 100);
    m_button[button_file_list]->setFixedSize(btnSize / 5 * 3);

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
    layout->addWidget(m_button[button_mode]);
    layout->addStretch();
    layout->addWidget(m_label[label_rate]);
    layout->addSpacing(10);
    layout->addWidget(m_button[button_file_list]);
}

void QPlayControl::initConnect()
{
    auto control = VIDEO_CONTROL;
    auto fileList = m_parent->findChild<QFileView*>();
    auto toolWidget = qobject_cast<QToolWidgets*>(m_parent);
    connect(control, &QVideoControl::total, this, &QPlayControl::onTotal);
    connect(control, &QVideoControl::frameRate, this, &QPlayControl::onRate);
    connect(control, &QVideoControl::end, this, &QPlayControl::onStop);
    connect(control, &QVideoControl::setPos, this, &QPlayControl::onUpdateCurrentTime);

    connect(m_button[button_play_or_continue], &QAbstractButton::released, this, &QPlayControl::onPlayOrPause);
    connect(m_button[button_stop], &QAbstractButton::clicked, fileList, &QFileView::onHandleStop);
    connect(m_button[button_stop], &QAbstractButton::clicked, control, &QVideoControl::onStoped);
    connect(m_button[button_vol_mute], &QAbstractButton::clicked, this, &QPlayControl::onMute);
    connect(m_button[button_file_list], &QAbstractButton::clicked, fileList, &QFileView::onAutoShow);
    connect(m_button[button_prev], &QAbstractButton::clicked, fileList, &QFileView::onHandleStop);
    connect(m_button[button_prev], &QAbstractButton::clicked, fileList, &QFileView::onPrev);
    connect(m_button[button_next], &QAbstractButton::clicked, fileList, &QFileView::onHandleStop);
    connect(m_button[button_next], &QAbstractButton::clicked, fileList, &QFileView::onNext);
    connect(m_button[button_mode], &QAbstractButton::clicked, this, &QPlayControl::onMode);

    connect(Config::instance(), &Config::loadConfig, this, &QPlayControl::onLoadConfig);
    connect(Config::instance(), &Config::setConfig, this, &QPlayControl::onSetConfig);

    connect(this, &QPlayControl::load, toolWidget, &QToolWidgets::onLoadFile);
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

    emit load();
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
    auto control = VIDEO_CONTROL;
    control->onSetMute(bChecked);
    SET_CONFIG_DATA(bChecked, Config::Data_Mute);
    m_button[button_vol_mute]->setToolTip(bChecked ? tr("close mute") : tr("set mute"));
}

void QPlayControl::onRate(int frameCount)
{
    auto label = m_label[label_rate];
    label->setText(QString("%1: %2/s").arg("fps").arg(frameCount));
}

void QPlayControl::onLoadConfig()
{
    auto mute = m_button[button_vol_mute];
    auto isMute = GET_CONFIG_DATA(Config::Data_Mute).toBool();
    mute->setChecked(isMute);
    if(isMute) onMute(isMute);

    onSetConfig();
}

void QPlayControl::onSetConfig()
{
    auto cur = GET_CONFIG_DATA(Config::Data_PlayMode).toInt();
    if(m_button[button_mode]->property("mode").toInt() != cur)
    {
        m_button[button_mode]->setProperty("mode", QString::number(cur));
        UTIL->flush(m_button[button_mode]);
        switch (cur) {
        case QPlayMenu::play_mode_loop:
            m_button[button_mode]->setToolTip(tr("order mode"));
            break;
        case QPlayMenu::play_mode_single:
            m_button[button_mode]->setToolTip(tr("single mode"));
            break;
        case QPlayMenu::play_mode_random:
            m_button[button_mode]->setToolTip(tr("random mode"));
            break;
        default:
            m_button[button_mode]->setToolTip("");
            break;
        }
    }
}

void QPlayControl::onTotal(int nTotal)
{
    m_nTotal = nTotal;
    m_sTotal = QTime::fromMSecsSinceStartOfDay(m_nTotal).toString("HH:mm:ss");
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
        auto cur = QTime::fromMSecsSinceStartOfDay(nSeconds).toString("HH:mm:ss");
        time->setText(QString("%1 / %2").arg(cur).arg(m_sTotal));
    }
}

void QPlayControl::onMode()
{
    auto cur = GET_CONFIG_DATA(Config::Data_PlayMode).toInt();
    cur = (cur + 1) % QPlayMenu::play_mode_max;
    SET_CONFIG_DATA(cur, Config::Data_PlayMode);
    auto output = m_parent->findChild<QOutputWidget*>();
    output->onInfo(m_button[button_mode]->toolTip());
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
        default:
            break;
        }
    }

    return QToolBase::eventFilter(watched, event);
}
