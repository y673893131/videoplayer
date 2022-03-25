#include "qwintaskbarmenu.h"

#if WIN32
#include <tchar.h>
#include <QVector>
#include <QDebug>
#include <QTimer>
#include <QFileInfo>
#include <QIcon>
#include <QFileIconProvider>
#include <QtWinExtras>
#include "Log/Log.h"
#include "qtsingleapplication/qtsingleapplication.h"
#include "control/videocontrol.h"
#include "qwinthumbnail.h"
#include "config/config.h"

class QWinTaskbarMenuPrivate : public VP_Data<QWinTaskbarMenu>
{
    VP_DECLARE_PUBLIC(QWinTaskbarMenu)
    inline QWinTaskbarMenuPrivate(QWinTaskbarMenu* parent)
        : VP_Data(parent)
        , m_sProID("vplay.taskbar.customJumpListProgID")
    {
        auto recent = m_jumplist.recent();
        recent->setVisible(true);
        recent->setTitle(QWinTaskbarMenu::tr("histroy"));
        auto tasks = m_jumplist.tasks();
        tasks->setTitle(QWinTaskbarMenu::tr("operator"));
        tasks->setVisible(true);

        if(/*tasks->isEmpty() || tasks->count() != cmd_type_max*/true)
        {
            QStringList titles, icons;

            titles << QWinTaskbarMenu::tr("prev");
            titles << QWinTaskbarMenu::tr("next");
            titles << QWinTaskbarMenu::tr("clean");
            titles << QWinTaskbarMenu::tr("stop");

            icons << ":/tool/prev_black";
            icons << ":/tool/next_black";
            icons << ":/title/close_hover";
            icons << ":/tool/stop_hover";

            for(int n = 0; n < titles.size(); ++n)
            {
                auto pair = qMakePair(titles[n], QString("control|%1").arg(titles[n]));
                m_tasks.push_back(pair);
                tasks->addLink(QIcon(icons[n]), pair.first, qApp->applicationFilePath(), QStringList(pair.second));
            }
        }

        initMsg();
    }

    ~QWinTaskbarMenuPrivate()
    {
    }

    void addPlay(const QString&, const QString&);
    void addTask(const QString&, const QString&);

    void clean();

    void initMsg();
    void msg(const QString&);
private:
    QString m_sProID;
    QVector<QString> m_fileTypes;
    QVector<QPair<QString, QString>> m_plays;
    QVector<QPair<QString, QString>> m_tasks;

    QWinJumpList m_jumplist;
};

void QWinTaskbarMenuPrivate::addTask(const QString & title, const QString & cmd)
{
    m_tasks.push_back(qMakePair(title, cmd));
}

void QWinTaskbarMenuPrivate::clean()
{
    QWinJumpList jumplist;
    auto recent = jumplist.recent();
    recent->clear();
    auto tasks = jumplist.tasks();
    tasks->clear();
    qDebug() << __FUNCTION__;
}

void QWinTaskbarMenuPrivate::initMsg()
{
    VP_Q(QWinTaskbarMenu);
    auto app = reinterpret_cast<QtSingleApplication*>(qApp);
    q->connect(app, &QtSingleApplication::messageReceived, q, [=](const QString& msg){ this->msg(msg); });

    QTimer::singleShot(0, [=]{
        VP_Q(QWinTaskbarMenu);
        auto control = VIDEO_CONTROL;
        q->connect(control, &QVideoControl::play, q, &QWinTaskbarMenu::onPlay);
    });
}

void QWinTaskbarMenuPrivate::msg(const QString &msg)
{
    VP_Q(QWinTaskbarMenu);
    Log(Log_Debug, "%s", msg.toLocal8Bit().data());
    auto list = msg.split('|');
    enum cmd_args
    {
        cmd_args_path,
        cmd_args_cmd,
        cmd_args_para,

        cmd_args_max
    };

    if(list.size() < cmd_args_max)
        return;

    auto args = list[cmd_args_para];
    auto cmd = list[cmd_args_cmd];
    if(cmd == "control")
    {
        int type = -1;
        for(int n = 0; n < m_tasks.size(); ++n)
        {
            if(msg.contains(m_tasks[n].second))
            {
                type = n;
                break;
            }
        }

        if(type < 0)
            return;

        if(type == cmd_type_clean)
        {
            auto recent = m_jumplist.recent();
            recent->clear();
        }

        Q_EMIT q->cmd(type, "");
    }
    else if(cmd == "play")
    {
        Q_EMIT q->cmd(cmd_type_play, args);
    }
}

QWinTaskbarMenu::QWinTaskbarMenu(QObject* parent)
    : QObject(parent)
    , VP_INIT(new QWinTaskbarMenuPrivate(this))
{
    QTimer::singleShot(0, [this]{
        VP_D(const QWinTaskbarMenu);
        auto func = [=]{
            auto bRecent = GET_CONFIG_DATA(Config::Data_Recent).toBool();
            if(!bRecent)
                d->m_jumplist.recent()->clear();
        };
        connect(Config::instance(), &Config::loadConfig, this, func);
        connect(Config::instance(), &Config::setConfig, this, func);
    });
}

void QWinTaskbarMenu::onPlay(const QString &file)
{
    VP_D(QWinTaskbarMenu);

    auto bRecent = GET_CONFIG_DATA(Config::Data_Recent).toBool();
    if(!bRecent)
        return;

    QFileInfo info(file);
    if(!info.isFile())
        return;

    QString sCmd = QString("play|%1").arg(file);
    QStringList args;
    args << sCmd;

    QFileIconProvider fp;
    auto recent = d->m_jumplist.recent();
//    bool bContain = false;
//    auto items = recent->items();
//    for(auto&& item : qAsConst(items))
//    {
//        if(item->description() == file)
//        {
//            bContain = true;
//            break;
//        }
//    }

//    if(recent->count() >= 5 || bContain)
//    {
//        QList<QWinJumpListItem *> itemsNew;
//        QWinJumpListItem* containItem = nullptr;
//        for(auto&& item : qAsConst(items))
//        {
//            auto itemNew = new QWinJumpListItem(QWinJumpListItem::Link);
//            itemNew->setIcon(item->icon());
//            itemNew->setTitle(item->title());
//            itemNew->setFilePath(item->filePath());
//            itemNew->setArguments(item->arguments());
//            itemNew->setDescription(item->description());
//            if(item->arguments()[0].contains(args[0]))
//            {
//                containItem = itemNew;
//            }
//            else
//            {
//                itemsNew.push_back(itemNew);
//            }
//        }

//        if(containItem)
//        {
//            itemsNew.push_back(containItem);
//        }

//        recent->clear();

//        if(!bContain)
//        {
//            itemsNew.removeFirst();
//        }

//        for(auto&& item : qAsConst(itemsNew))
//        {
//            recent->addItem(item);
//        }

//        if(bContain)
//        {
//            qDebug() << __FUNCTION__ << recent->count() << file << recent->items()[recent->count() - 1]->description();
//            return;
//        }
//    }

    auto item = recent->addLink(fp.icon(info), info.fileName(), qApp->applicationFilePath(), args);
    item->setDescription(file);
    qDebug() << __FUNCTION__ << recent->count() << file << recent->items()[recent->count() - 1]->description();
}

#endif
