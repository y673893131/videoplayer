#include "qplaymenu.h"
#include <QActionGroup>
#include "control/videocontrol.h"
#include "config/config.h"
#include "ui/qtoolwidgets.h"
#include "ui/qinputurlwidget.h"
#include "ui/tool/subtitle/qplaysubtitle.h"

QPlayMenu::QPlayMenu(QWidget *toolwidget, QWidget *parent)
    :QPlayMenuBase(parent)
    ,m_parent(toolwidget)
    ,m_nCurDecoder(0)
{
    init();
}

void QPlayMenu::init()
{
    initBaseAction();
    addSeparator();
    initSubMenu();
    prepareData();
}

void QPlayMenu::initBaseAction()
{
    m_actions[action_adjust] = addAction(tr("adjust"));
    m_actions[action_top_window] = addAction(tr("topWindow"));
    m_actions[action_url] = addAction(tr("url"));
    m_actions[action_capture] = addAction(tr("capture"));
    for(int n = 0; n < action_max; ++n)
    {
        m_actions[n]->setCheckable(true);
    }

    m_actions[action_url]->setCheckable(false);
}

void QPlayMenu::initSubMenu()
{
    QStringList menus;
    menus << tr("soundtrack");
    menus << tr("render");
    menus << tr("channel");
    menus << tr("decoder");
    menus << tr("speed");
    menus << tr("play");

    for(int n = 0; n < menu_max; ++n)
    {
        m_menus[n] = addMenu(menus[n]);
        m_menus[n]->setObjectName("menu1");
    }

    initSubMenuAction();
}

void QPlayMenu::initSubMenuAction()
{
    initSoundTrackActions();
    initRenderActions();
    initChannelSubMenu();
    initDecoderActions();
    initSpeedActions();
    initPlayActions();
}

void QPlayMenu::initSubMenuActions(const QStringList & sActions, QPlayMenu::menu index
                                   , bool bCheckedalbe, bool bNeedGroup)
{
    m_group[index] = new QActionGroup(this);
    for (auto it : sActions)
    {
        auto action = m_menus[index]->addAction(it);
        action->setCheckable(bCheckedalbe);
        if(bNeedGroup) m_group[index]->addAction(action);
    }
}

void QPlayMenu::initSoundTrackActions()
{
    QStringList sActions;
    sActions << tr("both") << tr("left") << tr("right");
    initSubMenuActions(sActions, menu_sound_track);
    m_group[menu_sound_track]->actions()[audio_channel_both]->setChecked(true);
}

void QPlayMenu::initRenderActions()
{
    QStringList sActions;
    sActions << tr("opengl") << tr("dx11");
    initSubMenuActions(sActions, menu_render);
}

void QPlayMenu::initDecoderActions()
{
    m_group[menu_decoder] = new QActionGroup(this);

    //type is hwcontext.h enum AVHWDeviceType
    QMap<int, QString> devs;
    devs.insert(2, "cuda");
    devs.insert(4, "dxva2");
    devs.insert(5, "qsv");
    devs.insert(7, "d3d11va");
    onSupport(devs);
}

void QPlayMenu::initSpeedActions()
{
    QStringList sActions;
    QString speed = tr("times");
    sActions << tr("normal") << ("0.5" + speed) << ("1.5" + speed) << ("2.0" + speed);
    initSubMenuActions(sActions, menu_speed);
    m_group[menu_speed]->actions()[0]->setChecked(true);
}

void QPlayMenu::initPlayActions()
{
    QStringList sActions;
    sActions << tr("loop play");
    initSubMenuActions(sActions, menu_play, true, false);
}

void QPlayMenu::initChannelSubMenu()
{
    m_channelMenus[channel_menu_video] = m_menus[menu_channel]->addMenu(tr("video"));
    m_channelMenus[channel_menu_audio] = m_menus[menu_channel]->addMenu(tr("audio"));
    m_channelMenus[channel_menu_subtitle] = m_menus[menu_channel]->addMenu(tr("subtitle"));
    for (int n = 0; n < channel_menu_max; ++n) {
        m_channelMenus[n]->setObjectName("menu1");
        m_channelGroup[n] = new QActionGroup(this);
    }
}

void QPlayMenu::initConnect()
{
    auto control = VIDEO_CONTROL;
    auto toolwidget = qobject_cast<QToolWidgets*>(m_parent);
    auto subtitle = m_parent->findChild<QPlaySubtitle*>();
    connect(Config::instance(), &Config::loadConfig, this, &QPlayMenu::onLoadConfig);
    connect(Config::instance(), &Config::setConfig, this, &QPlayMenu::onConfigChanged);

    connect(toolwidget, &QToolWidgets::showMenu, this, &QPlayMenu::onPop);

    connect(m_actions[action_adjust], &QAction::triggered, this, &QPlayMenu::onAdjustTriggered);
    connect(m_actions[action_adjust], &QAction::triggered, toolwidget, &QToolWidgets::viewAdjust);
    connect(m_actions[action_top_window], &QAction::triggered, toolwidget, &QToolWidgets::topWindow);
    connect(m_actions[action_url], &QAction::triggered, toolwidget, &QToolWidgets::inputUrl);
    connect(m_actions[action_capture], &QAction::triggered, control, &QVideoControl::onSetCapture);

    connect(m_group[menu_sound_track], &QActionGroup::triggered, this, &QPlayMenu::onSoundTrackTriggered);
    connect(m_group[menu_render], &QActionGroup::triggered, this, &QPlayMenu::onRenderTriggered);
    connect(m_group[menu_decoder], &QActionGroup::triggered, this, &QPlayMenu::onDecoderTriggered);
    connect(m_group[menu_speed], &QActionGroup::triggered, this, &QPlayMenu::onSpeedTriggered);
    connect(m_menus[menu_play], &QMenu::triggered, this, &QPlayMenu::onPlayTriggered);

    connect(this, &QPlayMenu::setDecodeType, control, &QVideoControl::onSetDecodeType);
    connect(this, &QPlayMenu::soundTrack, control, &QVideoControl::onSoundTrack);
    connect(this, &QPlayMenu::activeChannel, control, &QVideoControl::onActiveChannel);
    connect(this, &QPlayMenu::speed, control, &QVideoControl::onSpeed);
    connect(control, &QVideoControl::support, this, &QPlayMenu::onSupport);
    connect(control, &QVideoControl::streamInfo, this, &QPlayMenu::onStreamInfo);
    connect(control, &QVideoControl::end, this, &QPlayMenu::onEnd);

    connect(this, &QPlayMenu::subtitleModify, subtitle, &QPlaySubtitle::onChannelModify);

    for (int n = 0; n < channel_menu_max; ++n) {
        connect(m_channelGroup[n], &QActionGroup::triggered, this, &QPlayMenu::onChannelTriggered);
    }
}

void QPlayMenu::prepareData()
{
    m_actions[action_adjust]->setChecked(GET_CONFIG_DATA(Config::Data_Adjust).toBool());
    m_actions[action_top_window]->setChecked(GET_CONFIG_DATA(Config::Data_TopWindow).toBool());
}

void QPlayMenu::onLoadConfig()
{
    m_sCurRender = GET_CONFIG_DATA(Config::Data_Render).toString();
    for (auto ac : m_group[menu_render]->actions())
    {
        if(ac->text().contains(m_sCurRender))
        {
            ac->setChecked(true);
            break;
        }
    }

    m_nCurDecoder = GET_CONFIG_DATA(Config::Data_Decode).toInt();
    for(auto it : m_group[menu_decoder]->actions())
    {
        if(it->data().toInt() == m_nCurDecoder)
            it->setChecked(true);
    }

    emit setDecodeType(m_nCurDecoder);

//    m_actions[action_top_window]->setChecked(GET_CONFIG_DATA(Config::Data_TopWindow).toBool());
    m_menus[menu_play]->actions()[play_loop]->setChecked(GET_CONFIG_DATA(Config::Data_LoopPlay).toBool());
}

void QPlayMenu::onConfigChanged()
{
    m_actions[action_top_window]->setChecked(GET_CONFIG_DATA(Config::Data_TopWindow).toBool());
}

void QPlayMenu::onPop()
{
    popup(QCursor::pos());
}

void QPlayMenu::onSoundTrackTriggered(QAction *action)
{
    int sel = m_group[menu_sound_track]->actions().indexOf(action);
    emit soundTrack(sel);
}

void QPlayMenu::onRenderTriggered(QAction *action)
{
    for (auto ac : m_group[menu_render]->actions())
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
    auto nSel = m_menus[menu_speed]->actions().indexOf(action);
    emit speed(nSel);
}

void QPlayMenu::onPlayTriggered(QAction *action)
{
    auto nSel = m_menus[menu_play]->actions().indexOf(action);
    switch (nSel) {
    case play_loop:
         SET_CONFIG_DATA(action->isChecked(), Config::Data_LoopPlay);
        break;
    }
}

void QPlayMenu::onChannelTriggered(QAction *action)
{
    auto group = qobject_cast<QActionGroup*>(sender());
    int n = 0;
    for(;n < channel_menu_max; ++n)
    {
        if(m_channelGroup[n] == group)
            break;
    }

    auto index = group->actions().indexOf(action);
    emit activeChannel(n, index);
    if(n == channel_menu_subtitle)
    {
        emit subtitleModify();
    }
}

void QPlayMenu::onAdjustTriggered(bool bCheck)
{
    SET_CONFIG_DATA(bCheck, Config::Data_Adjust);
}

void QPlayMenu::onSupport(const QMap<int, QString> & devs)
{
    QStringList sActions;
    QString sSoftware = tr("software");
    QString sHardware = tr("hardware");

    for(auto it :m_group[menu_decoder]->actions())
        m_group[menu_decoder]->removeAction(it);

    auto action = m_group[menu_decoder]->addAction(QString("[%1]").arg(sSoftware));
    action->setData(0);
    action->setCheckable(true);


    for(QMap<int, QString>::const_iterator it = devs.begin(); it != devs.end(); ++it)
    {
        action = m_group[menu_decoder]->addAction(QString("[%1] %2").arg(sHardware).arg(it.value()));
        action->setData(it.key());
        action->setCheckable(true);
    }

    m_menus[menu_decoder]->clear();
    for(auto it : m_group[menu_decoder]->actions())
    {
        m_menus[menu_decoder]->addAction(it);
        if(m_nCurDecoder == it->data().toInt())
            it->setChecked(true);
    }
}

void QPlayMenu::onStreamInfo(const QStringList &list, int nChannel, int nDefault)
{
    auto actions = m_channelGroup[nChannel];
    auto menu = m_channelMenus[nChannel];
    menu->clear();
    for(auto ac : actions->actions())
        actions->removeAction(ac);
    if(nDefault < 0)
        return;
    int n = 0;
    for(auto it : list)
    {
        auto ac = menu->addAction(it);
        ac->setCheckable(true);
        actions->addAction(ac);
        if(n == nDefault)
            ac->setChecked(true);
        ++n;
    }

    menu->addActions(actions->actions());
}

void QPlayMenu::onEnd()
{
    for (int n = 0; n < channel_menu_max; ++n) {
        m_channelMenus[n]->clear();
        auto actions = m_channelGroup[n];
        for(auto ac : actions->actions())
            actions->removeAction(ac);
    }
}
