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

#define _OUTPUT_DEBUG

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
        waitClearRooms();

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
            room->sName = arrObj.value("nick").toString();
            room->sRn = arrObj.value("roomName").toString();
            room->sOd = arrObj.value("gameFullName").toString();
            m_room->append(room);
        }

        emit loadRoomsInfo(m_room->size());
    };

    auto request = new QNetworkRequest();
    request->setUrl(QUrl(url + QString::number(nPage)));
    appendRequest(request, func);
}

#include <QFile>
void QHuYaModel::getPreviewUrl(const QString &sRid)
{
    auto func = [=](QNetworkReply *response)
    {
        QString text = QString::fromUtf8(response->readAll());


        QRegExp re("\"stream\": \"(.*)/(.*)\"");
        re.setMinimal(true);
        re.indexIn(text);
        int n = 0;
        for(auto it : re.capturedTexts())
        {
            QString data = QString::fromUtf8(QByteArray::fromBase64(it.toUtf8()));

#ifdef _OUTPUT_DEBUG
        QFile f;
        f.setFileName(QString("D://%1.html").arg(n++));
        f.open(QFile::ReadWrite | QFile::Truncate);
        f.write(data.toUtf8());
        f.close();
        qDebug() << "[preview]" << data.length();
#endif

            QRegExp re1("\"sStreamName\":\"(.*)\"");
            re1.setMinimal(true);
            if(-1 == re1.indexIn(data))
                continue;

            auto realUrl = re1.cap(1);
            auto sLiveUrl = QString("rtmp://tx.flv.huya.com/src/%1").arg(realUrl);
            emit play(sLiveUrl);
            return;
        }
    };

    auto url = QString("https://www.huya.com/%1").arg(sRid);
#ifdef _OUTPUT_DEBUG
    qDebug() << "start get preview url:" << url;
#endif
    auto request = new QNetworkRequest();
    request->setUrl(QUrl(url));
    appendRequest(request, func);
}
