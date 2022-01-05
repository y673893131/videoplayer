#ifndef QVIDEOCONTROL_H
#define QVIDEOCONTROL_H

#include "video_player_core.h"
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
signals:
    void support(const QMap<int, QString>&);
    void start(int);
    void end(int);
    void total(int);
    void setPos(int);
    void videoSizeChanged(int, int);
    void frameRate(int);
    void appendFrame(void*);
    void subtitle(const QString&, unsigned int, int);
    void streamInfo(const QStringList&, int, int);
    void preview(void*, int, int);
    void play(const QString&);
    void info(const QString&);
    void jumpFailed();
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
    void setVideoSize(int width, int hight) override;
    void displayCall(void *data, int width, int height) override;
    void displayStreamChannelInfo(enum_stream_channel channel, const std::vector<_stream_channel_info_*>&, int defalut) override;
    void displaySubTitleCall(char *, unsigned int, int) override;
    void previewDisplayCall(void *data, int width, int height) override;
    void startCall(int) override;
    void endCall(int) override;
private:
    static QVideoControl* s_instance;
    video_player_core* m_core;
    QToolWidgets* m_toolbar;

    int m_frameCount, m_seekPos;
    QTimer* m_frameRateTimer;
    QTimer* m_seekTimer;
};

#endif // QVIDEOCONTROL_H
