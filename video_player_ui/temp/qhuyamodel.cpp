#include "qhuyamodel.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtXmlPatterns/QXmlQuery>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QScriptEngine>
#include <QCryptographicHash>
#include <QEventLoop>
QHuYaModel::QHuYaModel(QObject *parent)
    : QLiveDataModel(parent)
{
}

void QHuYaModel::initClass()
{
    // 加载大类
    auto func = [=](QNetworkReply *response)
    {
        QStringList names,urls;
        auto data = QString(response->readAll());
        QString sKey = "m-title j_sidebar-m-title";
        int nPos = data.indexOf(sKey);
        while(nPos > 0)
        {
            int headPos = nPos;
            _DyClass_ dyclass;
            auto titleName = getAttr(data, "m-title-span\">", "<", headPos);
            auto listBegin = data.indexOf("recomend-list clearfix", headPos);
            auto listEnd = data.indexOf("</div>", listBegin);
            nPos = data.indexOf("recomend-item j_sidebar-recomend-item", listBegin);
            while (listBegin < listEnd && listBegin > 0) {
                auto gameHref = getAttr(data, "href=\"", "\"", headPos);
                auto gameName = getAttr(data, "title=\"", "\"", headPos);
                listBegin = data.indexOf("recomend-item j_sidebar-recomend-item", headPos);
                if(listBegin < listEnd)
                    headPos = listBegin;
                dyclass.names << gameName;
                dyclass.urls << gameHref;
            }

            m_dy.nameToClass[titleName] = dyclass;
            m_dy.names << titleName;
            nPos = data.indexOf(sKey, nPos);
        }

        emit inited();
    };

    auto request = new QNetworkRequest();
    request->setUrl(QUrl("https://www.huya.com/l"));
    appendRequest(request, func);
}

void QHuYaModel::loadGameRooms(const QString& url)
{
    qDebug() << url;
    QString pgPath;
    auto list = url.split('/');
    if(list.size())
    {
        auto id = list[list.size() - 1];
        if(id == "lol")
            id = "1";
        pgPath = QString("https://www.huya.com/cache.php?m=LiveList&do=getLiveListByPage&gameId=%1&tagAll=0&page=").arg(id);
    }

    auto func = [=](QNetworkReply *response)
    {
        QString text = QString::fromUtf8(response->readAll());

        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        auto obj = doc.object();
        auto data = obj.value("data").toObject();
        auto pgCount = data["totalPage"].toInt();
//        qDebug() << pgCount << pgPath;
        emit totalPage(pgCount, pgPath);
        loadGameRoomPage(pgPath);
    };

    auto request = new QNetworkRequest();
    request->setUrl(QUrl(pgPath));
    appendRequest(request, func);
}

void QHuYaModel::loadGameRoomPage(const QString &url, int nPage)
{
    qDebug() << url << nPage;
    auto func = [=](QNetworkReply *response)
    {
        QString text = QString::fromUtf8(response->readAll());

        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        auto obj = doc.object();
        auto data = obj.value("data").toObject();
        auto arr = data.value("datas").toArray();
        QStringList sKeys;
        sKeys << "roomName" << "avatar180" << "screenshot" << "avatar180" << "url" << "nn";
//        qDebug() << arr.size() << obj["status"].toInt() << obj["message"].toString();
        emit m_room->clearRooms();
        QEventLoop loop;
        connect(m_room, &CRoomManager::clearRoomsed, &loop, &QEventLoop::quit);
        loop.exec();

        for(auto it : arr)
        {
            auto arrObj = it.toObject();
            auto room = new _DyRoom_();
            room->sRid = arrObj.value("profileRoom").toString();
            auto key = arrObj.value("uid").toString();
            room->sImg1 = arrObj.value("avatar180").toString();
            room->sImg2 = arrObj.value("screenshot").toString() + "?x-oss-process=image/resize,limit_0,m_fill,w_320,h_180/sharpen,80/format,jpg/interlace,1/quality,q_90";

            auto nPosBeg = room->sImg2.indexOf(key);
            auto nPosEnd = room->sImg2.indexOf("/", nPosBeg);
            auto uri = room->sImg2.mid(nPosBeg, nPosEnd - nPosBeg);
            room->sUrl = QString("/huya_%1").arg(room->sRid);
            if(!uri.isEmpty())
                room->sLiveUrl = QString("rtmp://tx.flv.huya.com/src/%1").arg(uri);
            room->nOnlineCount = arrObj.value("totalCount").toString().toInt();
//            room->sC2Name = arrObj.value("c2name").toString();

            room->sName = arrObj.value("nick").toString();
            room->sRn = arrObj.value("roomName").toString();
            room->sOd = arrObj.value("gameFullName").toString();
            m_room->append(room);
//            qDebug() << room->sRid << uri;
//            qDebug() << "-------room--------" << room->sRid << room->nOnlineCount << room->sName << room->sRn << room->sOd;
//            int n = 0;
//            for(auto key : sKeys)
//            {
//                if(!n)
//                    qDebug() << arrObj.value(key).toInt();
//                else
//                    qDebug() << arrObj.value(key).toString();
//                ++n;
//            }
        }

        emit loadRoomsInfo(m_room->size());
    };

    auto request = new QNetworkRequest();
    request->setUrl(QUrl(url + QString::number(nPage)));
    appendRequest(request, func);
}

void QHuYaModel::getPreviewUrl(const QString &sRid)
{
    auto func = [=](QNetworkReply *response)
    {
        QString text = QString::fromUtf8(response->readAll());
        QRegExp re("\"stream\": \"(.*)/(.*)==");
        re.setMinimal(true);
        re.indexIn(text);
        QString data = QString::fromUtf8(QByteArray::fromBase64(re.cap(2).toUtf8()));
        QRegExp re1("\"sStreamName\":\"(.*)\"");
        re1.setMinimal(true);
        re1.indexIn(data);
        auto sLiveUrl = QString("rtmp://tx.flv.huya.com/src/%1").arg(re1.cap(1));
        emit play(sLiveUrl);
    };

    auto url = QString("https://www.huya.com/%1").arg(sRid);
    auto request = new QNetworkRequest();
    request->setUrl(QUrl(url));
    appendRequest(request, func);
}