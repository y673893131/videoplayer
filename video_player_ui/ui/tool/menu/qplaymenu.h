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
        action_url,
        action_capture,

        action_max
    };

    enum play_action
    {
        play_loop,
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

public:
    explicit QPlayMenu(QWidget *toolwidget, QWidget *parent = nullptr);

    void initConnect();
private:
    void init();
    void initBaseAction();
    void initSubMenu();
    void initSubMenuAction();
    void initSubMenuActions(const QStringList&, menu, bool bCheckedalbe = true, bool bNeedGroup = true);
    void initSoundTrackActions();
    void initRenderActions();
    void initDecoderActions();
    void initSpeedActions();
    void initPlayActions();
    void initChannelSubMenu();
    void prepareData();
signals:
    void soundTrack(int);
    void setDecodeType(int);
    void activeChannel(int,int);
    void loadUrl();
    void subtitleModify();
    void speed(int);

private slots:
    void onLoadConfig();
    void onConfigChanged();
    void onPop();
    void onSoundTrackTriggered(QAction *action);
    void onRenderTriggered(QAction *action);
    void onDecoderTriggered(QAction *action);
    void onSpeedTriggered(QAction *action);
    void onPlayTriggered(QAction *action);
    void onChannelTriggered(QAction *action);

    void onAdjustTriggered(bool bCheck);
    void onSupport(const QMap<int, QString>&);
    void onStreamInfo(const QStringList &list, int nChannel, int nDefault);
    void onEnd();
private:
    QWidget* m_parent;
    QAction* m_actions[action_max];
    QMenu* m_menus[menu_max];
    QMenu* m_channelMenus[menu_max];

    QActionGroup* m_group[menu_max];
    QActionGroup* m_channelGroup[channel_menu_max];

    QString m_sCurRender;
    int m_nCurDecoder;
};

#endif // QPLAYMENU_H
