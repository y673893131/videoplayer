#include "videocontrol.h"
#include "render/videoframe.h"
#include "ui/qtoolwidgets.h"
#include "Log/Log.h"
#include <QApplication>
#include <QFileInfo>
#include <thread>
#include <QTime>
#include <QDebug>

QVideoControl::QVideoControl(QObject* parent)
    : QObject(parent)
    , m_frameCount(0)
    , m_seekPos(0)
{
    m_core = new video_player_core(qApp->applicationDirPath().toStdString());
    m_core->_init();
    m_core->_setCallBack(this);

    m_seekTimer = new QTimer;
    m_seekTimer->setInterval(200);
    m_seekTimer->setSingleShot(true);
    connect(m_seekTimer, &QTimer::timeout, this, &QVideoControl::onDoSeekPos, Qt::QueuedConnection);

    m_frameRateTimer = new QTimer;
    m_frameRateTimer->setInterval(1000);
    connect(m_frameRateTimer, &QTimer::timeout, this, &QVideoControl::onStatFrameRate, Qt::QueuedConnection);
    connect(this, &QVideoControl::start, m_frameRateTimer, static_cast<void(QTimer::*)()>(&QTimer::start), Qt::QueuedConnection);
    connect(this, &QVideoControl::end, m_frameRateTimer, &QTimer::stop, Qt::QueuedConnection);
}

void QVideoControl::setToolBar(QToolWidgets *toolWidget)
{
    m_toolbar = toolWidget;
    connect(m_toolbar, &QToolWidgets::play, this, &QVideoControl::onStart);
    connect(m_toolbar, &QToolWidgets::pause, this, &QVideoControl::onPause);
    connect(m_toolbar, &QToolWidgets::continuePlay, this, &QVideoControl::onContinue);
    connect(m_toolbar, &QToolWidgets::stop, this, &QVideoControl::onStoped);
    connect(m_toolbar, &QToolWidgets::setSeekPos, this, &QVideoControl::onSeekPos);
    connect(m_toolbar, &QToolWidgets::getPreview, this, &QVideoControl::onSeekPosImg);
    connect(m_toolbar, &QToolWidgets::setVol, this, &QVideoControl::onSetVol);
    connect(m_toolbar, &QToolWidgets::mute, this, &QVideoControl::onSetMute);
    connect(m_toolbar, &QToolWidgets::activeChannel, this, &QVideoControl::onActiveChannel);

    connect(this, &QVideoControl::start, m_toolbar, &QToolWidgets::start);
    connect(this, &QVideoControl::end, m_toolbar, &QToolWidgets::stop);
    connect(this, &QVideoControl::setPos, m_toolbar, &QToolWidgets::setPosSeconds);
    connect(this, &QVideoControl::total, m_toolbar, &QToolWidgets::setTotalSeconds);
    connect(this, &QVideoControl::frameRate, m_toolbar, &QToolWidgets::frameRate);
    connect(this, &QVideoControl::subtitle, m_toolbar, &QToolWidgets::onSubtitle);
    connect(this, &QVideoControl::streamInfo, m_toolbar, &QToolWidgets::onStreamInfo);
}

void QVideoControl::onStatFrameRate()
{
    emit frameRate(m_frameCount);
    m_frameCount = 0;
}

void QVideoControl::onStart(const QString &filename)
{
    if(filename.isEmpty())
    {
        emit m_toolbar->loadFile();
        return ;
    }

    if(m_core)
    {
        if(m_core->_getState(m_toolbar->index()) == video_player_core::state_paused)
        {
            qDebug() << "[countinue]";
            m_core->_continue(m_toolbar->index());
            return;
        }
    }

    waittingStoped();

    m_toolbar->setExists(QFileInfo::exists(filename));
    m_core->_setSrc(filename.toUtf8().toStdString());

    qDebug() << "[play]" << filename;
    Log(Log_Info, "[play] %s", filename.toLocal8Bit().toStdString().c_str());
    m_core->_play();
}

void QVideoControl::onStoped()
{
    m_core->_stop(m_toolbar->index());
}

void QVideoControl::onPause()
{
    m_core->_pause(m_toolbar->index());
    qDebug() << "[pause]" << m_toolbar->index();
}

void QVideoControl::onContinue()
{
    m_core->_continue(m_toolbar->index());
    qDebug() << "[continue]" << m_toolbar->index();
}

void QVideoControl::onSeekPos(int value)
{
    m_core->_seek(m_toolbar->index(), value);
//    m_seekPos = value;
//    m_seekTimer->stop();
//    m_seekTimer->start();
}

void QVideoControl::onDoSeekPos()
{
    m_core->_seek(m_toolbar->index(), m_seekPos);
}

void QVideoControl::onSeekPosImg(int value)
{
    m_core->_get_seek_img(m_toolbar->index(), value);
}

void QVideoControl::onSetVol(int value)
{
    m_core->_setVol(m_toolbar->index(), value);
}

void QVideoControl::onSetMute(bool bMute)
{
    m_core->_setMute(m_toolbar->index(), bMute);
}

void QVideoControl::onActiveChannel(int channel, int index)
{
    m_core->_setStreamChannel(m_toolbar->index(), channel, index);
}

void QVideoControl::waittingStoped()
{
    int index = m_toolbar->index();
    m_core->_stop(index);
    int state = m_core->_state(index);
    qDebug() << "[waitting] index:" << index << "state:" << state;
    while(state != video_player_core::state_uninit
           && state != video_player_core::state_stopped)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        state = m_core->_state(index);
    }
}

void QVideoControl::totalTime(const int64_t t)
{
    auto tm = QTime::fromMSecsSinceStartOfDay(static_cast<int>(t / 1000));
    qDebug() << "total:" << tm.toString("HH:mm:ss");
    emit total(static_cast<int>(t / 1000));
}

void QVideoControl::posChange(const int64_t t)
{
    emit setPos(static_cast<int>(t));
}

void QVideoControl::setVideoSize(int width, int height)
{
    emit videoSizeChanged(width, height);
}

void QVideoControl::displayCall(void *data, int width, int height)
{
    auto frame = new _video_frame_(reinterpret_cast<unsigned char*>(data), width, height);
    emit appendFrame(frame);
    ++m_frameCount;
}

void QVideoControl::displayStreamChannelInfo(enum_stream_channel channel, const std::vector<_stream_channel_info_ *>& infos, int defalut)
{
    QStringList list;
    int n = 0, select = 0;
    for(auto it : infos)
    {
        if(it->sTitle.empty())
            it->sTitle = tr("default").toStdString();
        if(it->sLanguage.empty())
            it->sLanguage = tr("default").toStdString();
        list << QString("%1 - [%2]").arg(QString::fromUtf8(it->sTitle.c_str())).arg(QString::fromUtf8(it->sLanguage.c_str()));
        if(it->index != defalut)
            ++n;
        else
            select = n;
    }

    emit streamInfo(list, static_cast<int>(channel), select);
}

void QVideoControl::displaySubTitleCall(char * str, int index)
{
//    qDebug() << QString::fromUtf8(str) << index;
    emit subtitle(QString::fromUtf8(str), index);
}

void QVideoControl::previewDisplayCall(void *data, int width, int height)
{
    emit m_toolbar->_preview(data, width, height);
}

void QVideoControl::startCall(int index)
{
    qDebug() << "[video] start:" << index;
    emit start(index);
}

void QVideoControl::endCall(int index)
{
    qDebug() << "[video] end:" << index;
    emit end(index);
}
