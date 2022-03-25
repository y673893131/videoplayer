#include "videocontrol.h"
#include "render/videoframe.h"
#include "ui/qtoolwidgets.h"
#include "Log/Log.h"
#include "config/config.h"
#include <QMetaType>
#include <QApplication>
#include <QFileInfo>
#include <thread>
#include <QTime>
#include <QDebug>

QVideoControl* QVideoControl::s_instance=nullptr;

QVideoControl* QVideoControl::instance()
{
    return s_instance;
}

QVideoControl::QVideoControl(QObject* parent)
    : QObject(parent)
    , m_index(0)
    , m_frameCount(0)
    , m_seekPos(0)
    , m_bitRate(0)
    , m_frameSize(-1,-1)
    , m_frame(new _video_frame_())
{
    setObjectName("video_control");

    qRegisterMetaType<_VideoFramePtr>("_VideoFramePtr");
    //const QMap<int, QString>&
    qRegisterMetaType<QMap<int, QString>>("QMap<int, QString>");
    qRegisterMetaType<QMap<int, QString>>("QMap<int, QString>&");
    qRegisterMetaType<int64_t>("int64_t");
    //subtitle_header
    qRegisterMetaType<subtitle_header>("subtitle_header");
    qRegisterMetaType<subtitle_header>("subtitle_header&");
    m_core = new video_player_core(qApp->applicationDirPath().toStdString());
    m_core->_init();
    m_core->_setCallBack(this);

    m_seekTimer = new QTimer;
    m_seekTimer->setInterval(200);
    m_seekTimer->setSingleShot(true);
    connect(m_seekTimer, &QTimer::timeout, this, &QVideoControl::onDoSeekPos, Qt::QueuedConnection);

    m_frameRateTimer = new QTimer;
    m_frameRateTimer->setInterval(1000);
    m_frameRateTimer->start();
    connect(m_frameRateTimer, &QTimer::timeout, this, &QVideoControl::onStatFrameRate, Qt::QueuedConnection);
    connect(this, &QVideoControl::start, m_frameRateTimer, static_cast<void(QTimer::*)()>(&QTimer::start), Qt::QueuedConnection);
    connect(this, &QVideoControl::end, m_frameRateTimer, &QTimer::stop, Qt::QueuedConnection);

    s_instance = this;
}

void QVideoControl::setToolBar(QToolWidgets *toolWidget)
{
    m_toolbar = toolWidget;
    connect(this, &QVideoControl::start, m_toolbar, &QToolWidgets::start);
    connect(this, &QVideoControl::total, m_toolbar, &QToolWidgets::setTotalSeconds);
}

void QVideoControl::onStatFrameRate()
{
//    static int count = 0;
//    Log(Log_Debug, "%s %d %d", __FUNCTION__, m_frameCount, count++);
    emit frameRate(m_frameCount);
    emit bitRateChanged(m_bitRate);
    m_frameCount = 0;
    m_bitRate = 0;
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
        onStoped();
        waittingStoped();
    }

    QFileInfo info(filename);
    m_toolbar->setExists(info.exists());
    if(info.isFile())
    {
        if(!info.exists())
        {
            emit tips(filename + tr("not exists."));
            emit notExist();
            return;
        }
    }

    m_core->_setSrc(filename.toUtf8().toStdString());

    Log(Log_Info, "[play] %s", filename.toLocal8Bit().toStdString().c_str());
    m_core->_play();
}

void QVideoControl::onStoped()
{
    m_core->_stop(m_index);
    m_bitRate = 0;
}

void QVideoControl::onPause()
{
    if(isPlaying())
        emit pausePlay();
    m_core->_pause(m_index);
    qDebug() << "[pause]" << m_index;
}

void QVideoControl::onContinue()
{
    if(isPause())
        emit continuePlay();
    m_core->_continue(m_index);
    qDebug() << "[continue]" << m_index;
}

void QVideoControl::onSeekPos(int value)
{
    m_core->_seek(m_index, value);
//    m_seekPos = value;
//    m_seekTimer->stop();
    //    m_seekTimer->start();
}

void QVideoControl::onJumpPos(int value)
{
    if(!m_core->_seekJump(m_index, value))
        emit jumpFailed();
}

void QVideoControl::onDoSeekPos()
{
    m_core->_seek(m_index, m_seekPos);
}

void QVideoControl::onSeekPosImg(int value)
{
    m_core->_get_seek_img(m_index, value);
}

void QVideoControl::onSetVol(int value)
{
    m_core->_setVol(m_index, value);
}

void QVideoControl::onSetMute(bool bMute)
{
    m_core->_setMute(m_index, bMute);
}

void QVideoControl::onActiveChannel(int channel, int index)
{
    m_core->_setStreamChannel(m_index, channel, index);
}

void QVideoControl::onSetDecodeType(int type)
{
    m_core->_setDecodeType(m_index, type);
}

void QVideoControl::onSetCapture(bool bCap)
{
    m_core->_setCapture(m_index, bCap);
}

void QVideoControl::onSoundTrack(int nSountTrack)
{
    m_core->_setAudioChannel(m_index, static_cast<audio_channel_type>(nSountTrack));
}

void QVideoControl::onSpeed(int type)
{
    m_core->_setSpeedType(m_index, type);
}

void QVideoControl::supportHWDecoder(const std::map<int, std::string> &devs)
{
    QMap<int, QString> tmps;
    for(auto it : devs)
    {
        tmps[it.first] = QString::fromStdString(it.second);
    }

    emit support(tmps);
}

void QVideoControl::waittingStoped()
{
    int index = m_index;
    m_core->_stop(index);
    int state = m_core->_state(index);
    qDebug() << "[waitting] index:" << index << "state:" << state;
    while(state != video_player_core::state_uninit
           && state != video_player_core::state_stopped)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        state = m_core->_state(index);
    }

    qDebug() << "[waitting] index:" << index << "done.";
}

bool QVideoControl::isPlaying()
{
    int index = m_index;
    int state = m_core->_state(index);
    return (state == video_player_core::state_running);
}

bool QVideoControl::isPause()
{
    int index = m_index;
    int state = m_core->_state(index);
    return (state == video_player_core::state_paused);
}

QSize QVideoControl::frameSize()
{
    return m_frameSize;
}

_VideoFramePtr QVideoControl::frame()
{
    return m_frame;
}

void *QVideoControl::buffer()
{
    return m_core->_frame(m_index);
}

void QVideoControl::totalTime(const int64_t t, const char* sName)
{
    auto tm = QTime::fromMSecsSinceStartOfDay(static_cast<int>(t / 1000));
    qDebug() << "total:" << tm.toString("HH:mm:ss");
    emit total(static_cast<int>(t / 1000));
    emit play(sName);
}

void QVideoControl::posChange(const int64_t t)
{
    emit setPos(static_cast<int>(t));
    ++m_frameCount;
}

void QVideoControl::bitRate(const int64_t size)
{
    m_bitRate += size;
}

void QVideoControl::setVideoSize(int width, int height)
{
    m_frameSize = QSize(width, height);
    emit videoSizeChanged(width, height);
}

void QVideoControl::displayCall(void *data, int width, int height)
{
    auto bZeroCopy = GET_CONFIG_DATA(Config::Data_ZeroCopy).toBool();
    if(bZeroCopy)
        m_frame->set(data, width, height);
    else
        m_frame = std::make_shared<_video_frame_>(reinterpret_cast<unsigned char*>(data), width, height);
    emit appendFrame(m_frame);
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

void QVideoControl::subtitleHaderCall(const subtitle_header &header)
{
    emit subTitleHeader(header);
}

void QVideoControl::displaySubTitleCall(char * str, unsigned int index, int type, int64_t start, int64_t end)
{
    emit subtitle(QString::fromUtf8(str), index, type, start, end);
}

void QVideoControl::displayFreqCall(float *data, unsigned int size)
{
//    auto d = new float[size];
//    memcpy(d, data, sizeof(float) * size);
    emit appendFreq(data, size);
}

void QVideoControl::previewDisplayCall(void *data, int width, int height)
{
    emit preview(data, width, height);
}

void QVideoControl::startCall(int index)
{
    qDebug() << "[video] start:" << index;
    m_index = index;
    emit start(index);
}

void QVideoControl::endCall(int index)
{
    qDebug() << "[video] end:" << index << "current" << m_index;
    if(index >= m_index)
        emit end(index);
}

void QVideoControl::exceptionEndCall(int index, const char* error)
{
    qDebug() << "[video] exception:" << index << "current" << m_index;
    if(index >= m_index)
    {
        emit exceptionEnd(index);
        emit tips(QString::fromUtf8(error));
    }
}
