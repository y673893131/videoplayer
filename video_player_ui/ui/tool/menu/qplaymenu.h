#ifndef QPLAYMENU_H
#define QPLAYMENU_H

#include "qplaymenubase.h"
#include "video_player_core.h"
#include "video_pimpl.h"

class QActionGroup;
class QPlayMenuPrivate;
class QPlayMenu : public QPlayMenuBase
{
    Q_OBJECT
    VP_DECLARE(QPlayMenu)
    VP_DECLARE_PRIVATE(QPlayMenu)
public:
    explicit QPlayMenu(QWidget *toolwidget, QWidget *parent = nullptr);
    ~QPlayMenu() override;

    void initConnect();

    enum play_mode
    {
        play_mode_loop,
        play_mode_single,
        play_mode_random,

        play_mode_max
    };

    enum play_size
    {
        play_size_video,
        play_size_window,
        play_size_16_9,
        play_size_4_3,

        play_size_max
    };

signals:
    void soundTrack(int);
    void setDecodeType(int);
    void activeChannel(int,int);
    void loadUrl();
    void subtitleModify();
    void subtitleEngineModify(int);
    void speed(int);
    void mode(int);

public slots:
    void onPop();
    void onLoadConfig();

private slots:
    void onConfigChanged();
    void onSoundTrackTriggered(QAction *action);
    void onRenderTriggered(QAction *action);
    void onDecoderTriggered(QAction *action);
    void onSpeedTriggered(QAction *action);
    void onSubtitleEngineTriggered(QAction *action);
    void onPlayModeTriggered(QAction *action);
    void onChannelTriggered(QAction *action);

    void onStreamInfo(const QStringList &list, int nChannel, int nDefault);
    void onEnd();
    void onPlayMode(int);
};

#endif // QPLAYMENU_H
