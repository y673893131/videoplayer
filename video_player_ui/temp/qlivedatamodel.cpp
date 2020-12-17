#include "qlivedatamodel.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtXmlPatterns/QXmlQuery>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QScriptEngine>
#include <QCryptographicHash>

QLiveDataModel::QLiveDataModel(QObject *parent) : QObject(parent)
  ,m_netWorkQueue(nullptr)
{
    m_sUserAgent = "Mozilla/5.0 (Linux; Android 5.0; SM-G900P Build/LRX21T) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/75.0.3770.100 Mobile Safari/537.36";
    m_netWorkQueue = new QNetWorkQueue();
    m_room = new CRoomManager();
    connect(this, &QLiveDataModel::loadRoomsInfo, m_room, &CRoomManager::flushRooms);
}

void QLiveDataModel::init()
{
    m_netWorkQueue->initRspThread();
    initClass();
}

int QLiveDataModel::classCount()
{
    return m_dy.names.size();
}

QString QLiveDataModel::className(int index)
{
    return m_dy.names[index];
}

int QLiveDataModel::gameCount(int classId)
{
    auto name = className(classId);
    return m_dy.nameToClass[name].names.size();
}

QString QLiveDataModel::gameName(int classId, int gameId)
{
    auto name = className(classId);
    return m_dy.nameToClass[name].names[gameId];
}

QString QLiveDataModel::gameUrl(int classId, int gameId)
{
    auto name = className(classId);
    return m_dy.nameToClass[name].urls[gameId];
}

void QLiveDataModel::initClass()
{
}

void QLiveDataModel::loadGameRooms(const QString& url)
{
    auto func = [=](QNetworkReply *response)
    {
        auto data = QString(response->readAll());

        int nPos = data.indexOf("\"pageCount\":");
        auto pgCount = getAttr(data, "\"pageCount\":", ",", nPos);
        auto pgPath = getAttr(data, "\"pagePath\":\"", "\"", nPos);
        qDebug() << pgCount << pgPath;
        emit totalPage(pgCount.toInt(), pgPath);
        loadGameRoomPage(pgPath);
    };

    auto request = new QNetworkRequest();
    request->setUrl(QUrl(QString("https://www.douyu.com") + url));
    request->setRawHeader("User-Agent", m_sUserAgent.toUtf8());
    appendRequest(request, func);
}

CRoomManager *QLiveDataModel::roomManager()
{
    return m_room;
}

void QLiveDataModel::loadGameRoomPage(const QString &/*url*/, int /*nPage*/)
{
}

void QLiveDataModel::getPreviewUrl(const QString & /*nRid*/)
{
}

QString QLiveDataModel::getAttr(const QString &data, const QString& attrBegin, const QString& attrEnd, int& pos)
{
    auto posBegin = data.indexOf(attrBegin, pos) + attrBegin.length();
    pos = data.indexOf(attrEnd, posBegin);
    return data.mid(posBegin, pos - posBegin);
}

void QLiveDataModel::appendRequest(QNetworkRequest * pReq, rsp_call call, const QByteArray &sData)
{
    m_netWorkQueue->appendRequest(pReq, call, sData);
}
