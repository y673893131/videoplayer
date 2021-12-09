#include "qegamemodel.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QDebug>

QEGameModel::QEGameModel(QObject* parent)
    :QLiveDataModel(parent)
{

}

void QEGameModel::loadGameRooms(const QString &url)
{
    auto list = url.split("layoutid=");
    if(list.size() != 2)
    {
        return;
    }

    QString pgPath = QString("https://share.egame.qq.com/cgi-bin/pgg_async_fcgi?param={\"key\":{\"module\":\"pgg_live_read_ifc_mt_svr\",\"method\":\"get_pc_live_list\",\"param\":{\"appid\":\"%1\",\"page_num\":%d,\"page_size\":40,\"tag_id\":0,\"tag_id_str\":\"\"}}}").arg(list[1]);
    QString sUrl = QString("https://share.egame.qq.com/cgi-bin/pgg_async_fcgi?param={\"key\":{\"module\":\"pgg_live_read_ifc_mt_svr\",\"method\":\"get_pc_live_list\",\"param\":{\"appid\":\"%1\",\"page_num\":1,\"page_size\":40,\"tag_id\":0,\"tag_id_str\":\"\"}}}").arg(list[1]);
    auto func = [=](QNetworkReply *response)
    {
        QString text = QString::fromUtf8(response->readAll());

        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        auto obj = doc.object();
        auto data = obj.value("data").toObject();
        auto key = data.value("key").toObject();
        auto retBody = key.value("retBody").toObject();
        data = retBody.value("data").toObject();
        auto perPageCount = 40;
        auto totalCount = data["total"].toInt();
        auto pgCount = totalCount / perPageCount + (totalCount % perPageCount == 0 ? 0 : 1);
        emit totalPage(pgCount, pgPath);
        loadGameRoomPage(pgPath);
    };

    auto request = new QNetworkRequest();
    request->setUrl(QUrl(sUrl));
    qDebug() << sUrl;
    appendRequest(request, func);
}

void QEGameModel::loadGameRoomPage(const QString &url, int pg)
{
    auto func = [=](QNetworkReply *response)
    {
        QString text = QString::fromUtf8(response->readAll());
        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        auto obj = doc.object();
        auto data = obj.value("data").toObject();
        auto key = data.value("key").toObject();
        auto retBody = key.value("retBody").toObject();
        auto data1 = retBody.value("data").toObject();
        auto liveData = data1.value("live_data").toObject();
        auto arr = liveData.value("live_list").toArray();
        waitClearRooms();

        for(auto it : arr)
        {
            auto arrObj = it.toObject();
            auto videoInfo = arrObj.value("video_info").toObject();
            auto room = new _DyRoom_();
            room->sRid = QString::number(arrObj.value("anchor_id").toInt());
            auto key = arrObj.value("uid").toString();
            room->sImg1 = videoInfo.value("url_high_reslution").toString();
            room->sImg2 = videoInfo.value("url").toString();
            room->sUrl = QString("/egame_%1").arg(room->sRid);
            room->nOnlineCount = arrObj.value("online").toInt();
            room->sName = arrObj.value("anchor_name").toString();
            room->sRn = arrObj.value("title").toString();
            room->sOd = arrObj.value("appname").toString();
            m_room->append(room);
        }

        emit loadRoomsInfo(m_room->size());
    };

    auto sUrl = QString::asprintf(url.toUtf8(), pg);
    auto request = new QNetworkRequest();
    request->setUrl(QUrl(sUrl));
    appendRequest(request, func);
}

void QEGameModel::getPreviewUrl(const QString & sRid)
{
    auto func = [=](QNetworkReply *response)
    {
        QString text = QString::fromUtf8(response->readAll());
        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        qDebug() << text.length() << error.errorString() << error.error;

        auto obj = doc.object();
        auto data = obj.value("data").toObject();
        auto key = data.value("key").toObject();
        auto retBody = key.value("retBody").toObject();
        auto data1 = retBody.value("data").toObject();
        auto videoInfo = data1.value("video_info").toObject();
        auto streamInfos = videoInfo.value("stream_infos").toArray();
        for(auto it : streamInfos)
        {
            auto itObj = it.toObject();
            emit play(itObj["play_url"].toString());
            break;
        }
    };

    auto sUrl = QString("https://share.egame.qq.com/cgi-bin/pgg_async_fcgi?param={\"key\":{\"module\":\"pgg_live_read_svr\",\"method\":\"get_live_and_profile_info\",\"param\":{\"anchor_id\":%1,\"layout_id\":\"\",\"index\":0}}}").arg(sRid);
    auto request = new QNetworkRequest();
    request->setUrl(QUrl(sUrl));
    appendRequest(request, func);
}

void QEGameModel::initClass()
{
    // 加载大类
    auto func = [=](QNetworkReply *response)
    {
        QStringList names,urls;
        auto data = QString(response->readAll());
        QString sKey = "<h4 class=\"game-channel\">";
        int nPos = data.indexOf(sKey);
        while(nPos > 0)
        {
            int headPos = nPos;
            _DyClass_ dyclass;
            auto titleName = getAttr(data, "game-channel\">", "<", headPos);
            auto listBegin = data.indexOf("<ul class=\"game-name\">", headPos);
            auto listEnd = data.indexOf("</a></li></ul></li>", listBegin);
            nPos = data.indexOf("class=\"text-white\"", listBegin);
            while (listBegin < listEnd && listBegin > 0) {
                auto gameHref = getAttr(data, "href=\"", "\"", headPos);
                auto gameName = getAttr(data, "\"text-white\">", "<", headPos);
                listBegin = data.indexOf("</li><li>", headPos);
                if(listBegin < listEnd)
                    headPos = listBegin;
                gameName = gameName.replace("\n", "");
                gameName = gameName.replace(" ", "");
                dyclass.names << gameName;
                dyclass.urls << gameHref;
            }

            titleName = titleName.replace("\n", "");
            titleName = titleName.replace(" ", "");
            m_dy.nameToClass[titleName] = dyclass;
            m_dy.names << titleName;
            nPos = data.indexOf(sKey, nPos);
        }

        emit inited();
    };

    auto request = new QNetworkRequest();
    request->setUrl(QUrl("https://egame.qq.com/livelist"));
    appendRequest(request, func);
}

