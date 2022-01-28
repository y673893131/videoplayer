#ifndef QPLAYMENU_H
#define QPLAYMENU_H

#include "qplaymenubase.h"
#include "video_player_core.h"

class QActionGroup;
class QPlayMenu : public QPlayMenuBase
{
    Q_OBJECT

public:
    enum action
    {
        action_adjust,
        action_top_window,
        action_capture,
        action_line0,
        action_url,
        action_line1,

        action_max
    };

    enum play_mode
    {
        play_mode_loop,
        play_mode_single,
        play_mode_random,

        play_mode_max
    };

    enum menu
    {
        menu_sound_track,
        menu_render,
        menu_channel,
        menu_decoder,
        menu_speed,
        menu_play,

        menu_max
    };

    enum channel_menu
    {
        channel_menu_video = channel_video,
        channel_menu_audio = channel_audio,
        channel_menu_subtitle = channel_subtitle,

        channel_menu_max
    };

    enum play_menu
    {
        play_menu_speed,

        play_menu_max
    };

public:
    explicit QPlayMenu(QWidget *toolwidget, QWidget *parent = nullptr);

    void initConnect();
private:
    void init();
    void prepareData();
signals:
    void soundTrack(int);
    void setDecodeType(int);
    void activeChannel(int,int);
    void loadUrl();
    void subtitleModify();
    void speed(int);
    void mode(int);

private slots:
    void onLoadConfig();
    void onConfigChanged();
    void onPop();
    void onSoundTrackTriggered(QAction *action);
    void onRenderTriggered(QAction *action);
    void onDecoderTriggered(QAction *action);
    void onSpeedTriggered(QAction *action);
    void onPlayModeTriggered(QAction *action);
    void onChannelTriggered(QAction *action);

    void onAdjustTriggered(bool bCheck);
    void onStreamInfo(const QStringList &list, int nChannel, int nDefault);
    void onEnd();
    void onPlayMode(int);
private:
    QWidget* m_parent;
    QString m_sCurRender;
    int m_nCurDecoder;
};

#endif // QPLAYMENU_H
