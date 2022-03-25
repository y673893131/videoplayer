#include "qdatamodel.h"
#include <QDebug>
#include <QMetaType>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFileInfo>
#include <QTimer>
#include "ui/tool/fileview/playlist/qplayfilelistmodel.h"
#include "config.h"
#include "Log/Log.h"
#include "configDef.h"

QDataModel *QDataModel::m_instance=nullptr;
QDataModel *QDataModel::instance()
{
    if(!m_instance)
    {
        m_instance = new QDataModel();
    }

    return m_instance;
}

QDataModel::QDataModel(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QVector<file_info_t>>("QVector<std::shared_ptr<file_info_t>>");
    qRegisterMetaType<QVector<file_info_t>>("QVector<std::shared_ptr<file_info_t>>&");
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("./data.tmp");
    connect(Config::instance(), &Config::setConfig, [this](const QString& config)
    {
        auto sql = QString("replace into config ('id', 'config') values (1, '%1');").arg(config);
        onExecSql(sql);
    });

    if(!m_db.open())
    {
        QMessageBox::warning(nullptr, tr("error"), tr("cannot load data."));
    }
    else
        initConfig();

    loadConfig();

    QTimer::singleShot(0, [this]
    {
        init();
    });
}

void QDataModel::init()
{
    QVector<std::shared_ptr<file_info_t>> data;
    QSqlQuery query;
    auto bquery = query.exec("select name, url,times from file_info; ");
    if(bquery)
    {
        auto record = query.record();
        while(query.next())
        {
            auto t = std::make_shared<file_info_t>();
            t->name = query.value(0).toString();
            t->url = query.value(1).toString();
            t->times = query.value(2).toInt();
            data.push_back(t);
        }
    }

    query.clear();
    emit loadsuccessed(data);
}

void QDataModel::onAddUrl(const QStringList &list)
{
    QString sFile;

    m_db.transaction();
    for(auto file : list)
    {
        if(!QFileInfo::exists(file)) continue;
        if(sFile.isEmpty()) sFile = file;
        auto name = file.mid(file.lastIndexOf('/') + 1);
        QSqlQuery query;
        auto sql = QString("insert into file_info ('name', 'url') values ('%1', '%2');").arg(name).arg(file);
        auto bquery = query.exec(sql);
        if(!bquery)
        {
            auto error = query.lastError();
            if(!error.text().contains("UNIQUE constraint failed"))
                reportError(query);
        }
        query.clear();
    }

    m_db.commit();

    init();

    emit addUrlSuccess(sFile);
}

void QDataModel::removeUrl(const QString &url)
{
    onExecSql(QString("delete from file_info where url='%1';").arg(url));
}

void QDataModel::onExecSql(const QString &sql)
{
    QSqlQuery query;
    if(!query.exec(sql))
        reportError(query);
    query.clear();
}

void QDataModel::onUpdateTimees(const QString &file, int times)
{
    onExecSql(QString("update file_info set times=%1 where url='%2';").arg(times).arg(file));
}

void QDataModel::onClean()
{
    onExecSql(QString("delete from file_info;"));
}

void QDataModel::initConfig()
{
    initTable("CREATE TABLE file_info ( \
            name VARCHAR(40) NOT NULL, \
            url VARCHAR(256) NOT NULL, \
            times int NOT NULL DEFAULT(''), \
            access_time TimeStamp NOT NULL DEFAULT(datetime('now','localtime')), PRIMARY KEY (name, url)) ");
    initTable("CREATE TABLE config (\
            id int NOT NULL DEFAULT(1),\
            config VARCHAR(1024) NOT NULL,\
            access_time TimeStamp NOT NULL DEFAULT(datetime('now','localtime')), PRIMARY KEY (id));");

//    onExecSql("alter table file_info add last_times int NOT NULL DEFAULT('')");
}

void QDataModel::initTable(const QString &sql)
{
    QSqlQuery query;
    auto bCreate = query.exec(sql);
    if(!bCreate)
    {
        auto error = query.lastError();
        qDebug() << error.text();
        if(!error.text().contains("already exists"))
            QMessageBox::warning(nullptr, tr("warning"), error.text() + "("+ error.nativeErrorCode() + ")");
    }

    query.clear();
}

void QDataModel::loadConfig()
{
    QSqlQuery query;
    auto bCreate = query.exec("select config from config limit 0, 1;");
    if(bCreate)
    {
        if(query.next())
        {
            Config::instance()->init(query.value(0));
        }
        else
        {
            Config::instance()->init("");
        }

        query.clear();
    }
    query.clear();
}

void QDataModel::reportError(const QSqlQuery & query)
{
    auto error = query.lastError();
    QMessageBox::warning(nullptr, tr("warning"), error.text() + "("+ error.nativeErrorCode() + ")");
}
