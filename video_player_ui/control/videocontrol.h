#ifndef QVIDEOCONTROL_H
#define QVIDEOCONTROL_H

#include "video_player_core.h"
#include "render/videoframe.h"
#include <QObject>
#include <QTimer>
#include <QApplication>

#define VIDEO_CONTROL QVideoControl::instance()

class QToolWidgets;
class QVideoControl : public QObject, public video_interface
{
    Q_OBJECT
public:
    static QVideoControl* instance();
    QVideoControl(QObject* parent = nullptr);

    void setToolBar(QToolWidgets* toolWidget);
    void waittingStoped();
    bool isPlaying();
    bool isPause();
    QSize frameSize();
    _VideoFramePtr frame();
    void *buffer();
signals:
    void support(const QMap<int, QString>&);
    void start(int);
    void exceptionEnd(int);
    void end(int);
    void total(int);
    void setPos(int);
    void videoSizeChanged(int, int);
    void frameRate(int);
    void bitRateChanged(int64_t);
    void appendFrame(_VideoFramePtr);
    void appendFreq(float*, unsigned int);
    void subTitleHeader(const subtitle_header&);
    void subtitle(const QString&, unsigned int, int, int64_t, int64_t);
    void streamInfo(const QStringList&, int, int);
    void preview(void*, int, int);
    void play(const QString&);
    void info(const QString&);
    void jumpFailed();
    void pausePlay();
    void continuePlay();
    void tips(const QString&);
    void notExist();
public slots:
    void onStatFrameRate();
    void onStart(const QString&);
    void onStoped();
    void onPause();
    void onContinue();
    void onSeekPos(int);
    void onJumpPos(int);
    void onDoSeekPos();
    void onSeekPosImg(int);
    void onSetVol(int);
    void onSetMute(bool);
    void onActiveChannel(int, int);
    void onSetDecodeType(int);
    void onSetCapture(bool bCap);
    void onSoundTrack(int);
    void onSpeed(int);
private:
    Q_DISABLE_COPY(QVideoControl)

    // video_interface interface
public:
    void supportHWDecoder(const std::map<int, std::string>&) override;
    void totalTime(const int64_t t, const char*) override;
    void posChange(const int64_t t) override;
    void bitRate(const int64_t size) override;
    void setVideoSize(int width, int hight) override;
    void displayCall(void *data, int width, int height) override;
    void displayStreamChannelInfo(enum_stream_channel channel, const std::vector<_stream_channel_info_*>&, int defalut) override;
    void subtitleHaderCall(const subtitle_header &) override;
    void displaySubTitleCall(char *, unsigned int, int, int64_t, int64_t) override;
    void displayFreqCall(float*,  unsigned int) override;
    void previewDisplayCall(void *data, int width, int height) override;
    void startCall(int) override;
    void endCall(int) override;
    void exceptionEndCall(int, const char*) override;
private:
    static QVideoControl* s_instance;
    video_player_core* m_core;
    QToolWidgets* m_toolbar;

    int m_index, m_frameCount, m_seekPos;
    int64_t m_bitRate;
    QSize m_frameSize;
    QTimer* m_frameRateTimer;
    QTimer* m_seekTimer;

    _VideoFramePtr m_frame;
};

#endif // QVIDEOCONTROL_H
