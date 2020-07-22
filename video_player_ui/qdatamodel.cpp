#include "qdatamodel.h"
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFileInfo>
QDataModel::QDataModel(QObject *parent) : QObject(parent), m_nVol(50)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("./data.tmp");
    if(!m_db.open())
    {
        QMessageBox::warning(nullptr, tr("error"), tr("cannot load data."));
    }
    else
        initConfig();
}

void QDataModel::init()
{
    QVector<QStringList> data;
    QSqlQuery query;
    auto bquery = query.exec("select name, url from file_info limit 0, 30; ");
    if(bquery)
    {
        auto record = query.record();
        while(query.next())
        {
            auto name = query.value(0).toString();
            auto url = query.value(1).toString();
            QStringList s;
            s << name << url;
            data.push_back(s);
        }
    }

    query.clear();
    emit loadsuccessed(data);
}

const QString &QDataModel::configPath()
{
    return m_sPath;
}

int QDataModel::configVol()
{
    return m_nVol;
}

void QDataModel::onAddUrl(const QString &file)
{
    if(!QFileInfo::exists(file)) return;
    auto name = file.mid(file.lastIndexOf('/') + 1);
    QSqlQuery query;
    auto sql = QString("insert into file_info ('name', 'url') values ('%1', '%2');").arg(name).arg(file);
    auto bquery = query.exec(sql);
    if(!bquery) {
        init();
        return;
    }

    bquery = query.exec(QString("delete from file_info where url not in (select url from file_info order by access_time desc limit 0, 30);"));
    if(!bquery) reportError(query);
    init();
    query.clear();
}

void QDataModel::initConfig()
{
    initTable("CREATE TABLE file_info ( \
              name VARCHAR(40) NOT NULL, \
              url VARCHAR(256) NOT NULL, \
              times int NOT NULL DEFAULT(''), \
              access_time TimeStamp NOT NULL DEFAULT(datetime('now','localtime')), PRIMARY KEY (name, url)) ");
    initTable("CREATE TABLE config (\
              path VARCHAR(255) NOT NULL,\
              vol int NOT NULL,\
              access_time TimeStamp NOT NULL DEFAULT(datetime('now','localtime')));");
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
    auto bCreate = query.exec("select path,vol from config limit 0, 1;");
    if(bCreate)
    {
        auto record = query.record();
        while(query.next())
        {
            m_sPath = query.value(0).toString();
            m_nVol = query.value(1).toInt();
        }
    }
    query.clear();
}

void QDataModel::reportError(const QSqlQuery & query)
{
    auto error = query.lastError();
    QMessageBox::warning(nullptr, tr("warning"), error.text() + "("+ error.nativeErrorCode() + ")");
}