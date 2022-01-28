#include "qplaymenu.h"
#include <QActionGroup>
#include "control/videocontrol.h"
#include "config/config.h"
#include "ui/qtoolwidgets.h"
#include "ui/qinputurlwidget.h"
#include "ui/tool/subtitle/qplaysubtitle.h"

QPlayMenu::QPlayMenu(QWidget *toolwidget, QWidget *parent)
    :QPlayMenuBase(":/xml/main_menu", parent)
    ,m_parent(toolwidget)
    ,m_nCurDecoder(0)
{
    init();
}

void QPlayMenu::init()
{
    prepareData();
}

void QPlayMenu::prepareData()
{
    auto actions = this->actions();

    actions[action_adjust]->setChecked(GET_CONFIG_DATA(Config::Data_Adjust).toBool());
    actions[action_top_window]->setChecked(GET_CONFIG_DATA(Config::Data_TopWindow).toBool());
    group("soundtrack")->actions()[audio_channel_both]->setChecked(true);
    group("speed")->actions()[0]->setChecked(true);
}

void QPlayMenu::initConnect()
{
    auto control = VIDEO_CONTROL;
    auto toolwidget = qobject_cast<QToolWidgets*>(m_parent);
    auto subtitle = m_parent->findChild<QPlaySubtitle*>();
    auto actions = this->actions();

    connect(Config::instance(), &Config::loadConfig, this, &QPlayMenu::onLoadConfig);
    connect(Config::instance(), &Config::setConfig, this, &QPlayMenu::onConfigChanged);

    connect(toolwidget, &QToolWidgets::showMenu, this, &QPlayMenu::onPop);

    connect(actions[action_adjust], &QAction::triggered, this, &QPlayMenu::onAdjustTriggered);
    connect(actions[action_adjust], &QAction::triggered, toolwidget, &QToolWidgets::viewAdjust);
    connect(actions[action_top_window], &QAction::triggered, toolwidget, &QToolWidgets::topWindow);
    connect(actions[action_capture], &QAction::triggered, control, &QVideoControl::onSetCapture);
    connect(actions[action_url], &QAction::triggered, toolwidget, &QToolWidgets::inputUrl);

    connect(subMenu("video"), &QMenu::triggered, this, &QPlayMenu::onChannelTriggered);
    connect(subMenu("audio"), &QMenu::triggered, this, &QPlayMenu::onChannelTriggered);
    connect(subMenu("subtitle"), &QMenu::triggered, this, &QPlayMenu::onChannelTriggered);
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
    connect(this, &QPlayMenu::mode, this, &QPlayMenu::onPlayMode);
}

void QPlayMenu::onLoadConfig()
{
    m_sCurRender = GET_CONFIG_DATA(Config::Data_Render).toString();
    auto actions = group("render")->actions();
    for(auto&& ac : qAsConst(actions))
    {
        if(ac->text().contains(m_sCurRender))
        {
            ac->setChecked(true);
            break;
        }
    }

    m_nCurDecoder = GET_CONFIG_DATA(Config::Data_Decode).toInt();
    actions = group("decoder")->actions();
    for(auto&& ac : qAsConst(actions))
    {
        if(ac->data().toInt() == m_nCurDecoder)
            ac->setChecked(true);
    }

    emit setDecodeType(m_nCurDecoder);

    onConfigChanged();
}

void QPlayMenu::onConfigChanged()
{
    if(GET_CONFIG_DATA(Config::Data_TopWindow).toBool() != actions()[action_top_window]->isChecked())
    {
        actions()[action_top_window]->setChecked(GET_CONFIG_DATA(Config::Data_TopWindow).toBool());
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
    auto actions = group("render")->actions();
    for(auto&& ac : qAsConst(actions))
    {
        if(!ac->text().contains(m_sCurRender) && action == ac)
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
    if(menuIndex == channel_menu_subtitle)
    {
        emit subtitleModify();
    }
}

void QPlayMenu::onAdjustTriggered(bool bCheck)
{
    SET_CONFIG_DATA(bCheck, Config::Data_Adjust);
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
