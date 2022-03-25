#include "qplaymenu.h"
#include <QActionGroup>
#include "control/videocontrol.h"
#include "config/config.h"
#include "ui/qtoolwidgets.h"
#include "ui/pop/qinputurlwidget.h"
#include "ui/tool/subtitle/qplaysubtitle.h"

class QPlayMenuPrivate : public VP_Data<QPlayMenu>
{
    VP_DECLARE_PUBLIC(QPlayMenu)
    inline QPlayMenuPrivate(QPlayMenu* parent, QWidget* grand)
        : VP_Data(parent)
        , m_parent(grand)
        , m_nCurDecoder(0)
    {
        init();
    }

    enum action
    {
        action_top_window,
        action_capture,
        action_line0,
        action_line1,

        action_max
    };

    enum menu
    {
        menu_pop,
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

    void init();
    void prepareData();
private:
    QWidget* m_parent;
    QString m_sCurRender;
    int m_nCurDecoder;
};

void QPlayMenuPrivate::init()
{
    prepareData();
}

void QPlayMenuPrivate::prepareData()
{
    VP_Q(QPlayMenu);
    auto actions = q->actions();

    actions[action_top_window]->setChecked(GET_CONFIG_DATA(Config::Data_TopWindow).toBool());
    q->group("soundtrack")->actions()[audio_channel_both]->setChecked(true);
    q->group("speed")->actions()[0]->setChecked(true);
}

QPlayMenu::QPlayMenu(QWidget *toolwidget, QWidget *parent)
    :QPlayMenuBase(":/xml/main_menu", parent)
    ,VP_INIT(new QPlayMenuPrivate(this, toolwidget))
{
}

QPlayMenu::~QPlayMenu()
{
}

void QPlayMenu::initConnect()
{
    VP_D(QPlayMenu);

    auto control = VIDEO_CONTROL;
    auto toolwidget = qobject_cast<QToolWidgets*>(d->m_parent);
    auto subtitle = d->m_parent->findChild<QPlaySubtitle*>();
    auto actions = this->actions();

    connect(Config::instance(), &Config::loadConfig, this, &QPlayMenu::onLoadConfig);
    connect(Config::instance(), &Config::setConfig, this, &QPlayMenu::onConfigChanged);

    connect(actions[d->action_top_window], &QAction::triggered, toolwidget, &QToolWidgets::topWindow);
    connect(actions[d->action_capture], &QAction::triggered, control, &QVideoControl::onSetCapture);

    connect(subMenu("video"), &QMenu::triggered, this, &QPlayMenu::onChannelTriggered);
    connect(subMenu("audio"), &QMenu::triggered, this, &QPlayMenu::onChannelTriggered);
    connect(subMenu("subtitle"), &QMenu::triggered, this, &QPlayMenu::onChannelTriggered);
    connect(subMenu("subtitle_eng"), &QMenu::triggered, this, &QPlayMenu::onSubtitleEngineTriggered);
    connect(subMenu("play_size"), &QMenu::triggered, this, [=](QAction* action){
        auto menu = qobject_cast<QMenu*>(sender());
        int index = menu->actions().indexOf(action);
        SET_CONFIG_DATA(index, Config::Data_PlaySize);
    });

    connect(subMenu("pop"), &QMenu::triggered, this, [=](QAction* action){
        emit toolwidget->pop(action->property("name").toString());
    });

    connect(subMenu("setting"), &QMenu::triggered, this, [=](QAction* action){
        if(action->property("name") == "recent")
        {
            SET_CONFIG_DATA(action->isChecked(), Config::Data_Recent);
        }
        else if(action->property("name") == "zero_copy")
        {
            SET_CONFIG_DATA(action->isChecked(), Config::Data_ZeroCopy);
        }
    });

    connect(group("soundtrack"), &QActionGroup::triggered, this, &QPlayMenu::onSoundTrackTriggered);
    connect(group("render"), &QActionGroup::triggered, this, &QPlayMenu::onRenderTriggered);
    connect(group("decoder"), &QActionGroup::triggered, this, &QPlayMenu::onDecoderTriggered);
    connect(group("speed"), &QActionGroup::triggered, this, &QPlayMenu::onSpeedTriggered);
    connect(group("mode"), &QActionGroup::triggered, this, &QPlayMenu::onPlayModeTriggered);

    connect(this, &QPlayMenu::setDecodeType, control, &QVideoControl::onSetDecodeType);
    connect(this, &QPlayMenu::soundTrack, control, &QVideoControl::onSoundTrack);
    connect(this, &QPlayMenu::activeChannel, control, &QVideoControl::onActiveChannel);
    connect(this, &QPlayMenu::speed, control, &QVideoControl::onSpeed);
    connect(control, &QVideoControl::streamInfo, this, &QPlayMenu::onStreamInfo);
    connect(control, &QVideoControl::end, this, &QPlayMenu::onEnd);

    connect(this, &QPlayMenu::subtitleModify, subtitle, &QPlaySubtitle::onChannelModify);
    connect(this, &QPlayMenu::subtitleEngineModify, subtitle, &QPlaySubtitle::onSetEngine);
    connect(this, &QPlayMenu::mode, this, &QPlayMenu::onPlayMode);
}

void QPlayMenu::onLoadConfig()
{
    VP_D(QPlayMenu);
    d->m_sCurRender = GET_CONFIG_DATA(Config::Data_Render).toString();
    auto actions = group("render")->actions();
    for(auto&& ac : qAsConst(actions))
    {
        if(ac->text().contains(d->m_sCurRender))
        {
            ac->setChecked(true);
            break;
        }
    }

    d->m_nCurDecoder = GET_CONFIG_DATA(Config::Data_Decode).toInt();
    actions = group("decoder")->actions();
    for(auto&& ac : qAsConst(actions))
    {
        if(ac->data().toInt() == d->m_nCurDecoder)
            ac->setChecked(true);
    }

    emit setDecodeType(d->m_nCurDecoder);

    subMenu("subtitle_eng")->actions()[0]->setChecked(true);

    auto playSize = GET_CONFIG_DATA(Config::Data_PlaySize).toInt();
    subMenu("play_size")->actions()[playSize]->setChecked(true);

    auto bRecent = GET_CONFIG_DATA(Config::Data_Recent).toBool();
    action("setting", "recent")->setChecked(bRecent);

    auto bZeroCopy = GET_CONFIG_DATA(Config::Data_ZeroCopy).toBool();
    action("setting", "zero_copy")->setChecked(bZeroCopy);

    onConfigChanged();
}

void QPlayMenu::onConfigChanged()
{
    if(GET_CONFIG_DATA(Config::Data_TopWindow).toBool() != actions()[QPlayMenuPrivate::action_top_window]->isChecked())
    {
        actions()[QPlayMenuPrivate::action_top_window]->setChecked(GET_CONFIG_DATA(Config::Data_TopWindow).toBool());
    }

    auto curMode = GET_CONFIG_DATA(Config::Data_PlayMode).toInt();
    group("mode")->actions()[curMode]->setChecked(true);
    onPlayMode(curMode);
}

void QPlayMenu::onPop()
{
    popup(QCursor::pos());
}

void QPlayMenu::onSoundTrackTriggered(QAction *action)
{
    int sel = group("soundtrack")->actions().indexOf(action);
    emit soundTrack(sel);
}

void QPlayMenu::onRenderTriggered(QAction *action)
{
    VP_D(QPlayMenu);
    auto actions = group("render")->actions();
    for(auto&& ac : qAsConst(actions))
    {
        if(!ac->text().contains(d->m_sCurRender) && action == ac)
        {
            ac->setText(ac->text() + tr("(restart valid)"));
        }
        else
        {
            ac->setText(ac->text().replace(tr("(restart valid)"), ""));
        }
    }

    action->setChecked(true);
    SET_CONFIG_DATA(action->text().replace(tr("(restart valid)"), ""), Config::Data_Render);
}

void QPlayMenu::onDecoderTriggered(QAction *action)
{
    int sel = action->data().toInt();
    action->setChecked(true);
    SET_CONFIG_DATA(sel, Config::Data_Decode);
    emit setDecodeType(sel);
}

void QPlayMenu::onSpeedTriggered(QAction *action)
{
    auto nSel = subMenu("speed")->actions().indexOf(action);
    emit speed(nSel);
}

void QPlayMenu::onSubtitleEngineTriggered(QAction *action)
{
    auto nSel = subMenu("subtitle_eng")->actions().indexOf(action);
    emit subtitleEngineModify(nSel);
}

void QPlayMenu::onPlayModeTriggered(QAction *action)
{
    auto nSel = group("mode")->actions().indexOf(action);
    SET_CONFIG_DATA(nSel, Config::Data_PlayMode);
    emit mode(nSel);
}

void QPlayMenu::onChannelTriggered(QAction *action)
{
    auto menu = qobject_cast<QMenu*>(sender());
    auto data = menu->property("data").toString();
    auto menuIndex = data.toInt();
    auto actionIndex = menu->actions().indexOf(action);
    emit activeChannel(menuIndex, actionIndex);
    if(menuIndex == QPlayMenuPrivate::channel_menu_subtitle)
    {
        emit subtitleModify();
    }
}

void QPlayMenu::onStreamInfo(const QStringList &list, int nChannel, int nDefault)
{
    QStringList names;
    names << "video" << "audio" << "subtitle";
    auto group = this->group(names[nChannel]);
    auto menu = subMenu(names[nChannel]);
    menu->clear();
    auto actions = group->actions();
    for(auto&& ac : qAsConst(actions))
        group->removeAction(ac);
    if(nDefault < 0)
        return;
    int n = 0;
    for(auto&& it : qAsConst(list))
    {
        auto ac = menu->addAction(it);
        ac->setCheckable(true);
        group->addAction(ac);
        if(n == nDefault)
            ac->setChecked(true);
        ++n;
    }

    menu->addActions(group->actions());
}

void QPlayMenu::onEnd()
{
    QStringList names;
    names << "video" << "audio" << "subtitle";
    for(auto&& name : qAsConst(names))
    {
        subMenu(name)->clear();
        auto group = this->group(name);
        auto actions = this->group(name)->actions();
        for(auto&& ac : qAsConst(actions))
            group->removeAction(ac);
    }
}

void QPlayMenu::onPlayMode(int mode)
{
    auto menu = subMenu("mode");
    QIcon icon;
    switch (mode) {
    case play_mode_loop:
        icon.addFile(":/menu/mode_normal", QSize(), QIcon::Mode::Normal);
        icon.addFile(":/menu/mode_normal_hover", QSize(), QIcon::Mode::Active);
        break;
    case play_mode_single:
        icon.addFile(":/menu/mode_single", QSize(), QIcon::Mode::Normal);
        icon.addFile(":/menu/mode_single_hover", QSize(), QIcon::Mode::Active);
        break;
    case play_mode_random:
        icon.addFile(":/menu/mode_random", QSize(), QIcon::Mode::Normal);
        icon.addFile(":/menu/mode_random_hover", QSize(), QIcon::Mode::Active);
        break;
    }

    menu->setIcon(icon);
}
