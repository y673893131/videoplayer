#include "qdymodel.h"
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

QDyModel::QDyModel(QObject *parent)
    : QLiveDataModel(parent)
{
}

void QDyModel::initClass()
{
    // 加载大类
    auto func = [=](QNetworkReply *response)
    {
        QStringList names,urls;
        auto data = QString(response->readAll());
        QString sKey = "Aside-menu-block";
        int nPos = data.indexOf(sKey);
        while(nPos > 0)
        {
            int headPos = data.indexOf("Aside-menu-head", nPos);
            if(headPos < 0)
                break;
            headPos = data.indexOf("Aside-menu-title", headPos);

            _DyClass_ dyclass;
            auto titleHref = getAttr(data, "href=\"", "\"", headPos);
            auto titleName = getAttr(data, ">", "<", headPos);

            auto listBegin = data.indexOf("Aside-menu-list", headPos);
            auto listEnd = data.indexOf("</div>", listBegin);
            nPos = data.indexOf("Aside-menu-item", listBegin);
            while (listBegin < listEnd && listBegin > 0) {
                auto gameHref = getAttr(data, "href=\"", "\"", headPos);
                auto gameName = getAttr(data, "title=\"", "\"", headPos);
                listBegin = data.indexOf("Aside-menu-item", headPos);
                if(listBegin < listEnd)
                    headPos = listBegin;
                dyclass.names << gameName;
                dyclass.urls << gameHref;
            }

            m_dy.nameToClass[titleName] = dyclass;
            m_dy.names << titleName;
            m_dy.urls << titleHref;
        }

        emit inited();
    };

    connect(this, &QDyModel::inited, this, &QDyModel::getVerArg);
    connect(this, &QDyModel::loadVered, this, [=]{
    });

    auto request = new QNetworkRequest();
    request->setUrl(QUrl("https://www.douyu.com/directory/all"));
    request->setRawHeader("User-Agent", m_sUserAgent.toUtf8());
    appendRequest(request, func);
}

void QDyModel::loadGameRooms(const QString& url)
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

void QDyModel::loadGameRoomPage(const QString &url, int nPage)
{
    auto func = [=](QNetworkReply *response)
    {
        QString text = response->readAll();

        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        auto obj = doc.object();
        auto data = obj.value("data");
        auto arr = data.toObject().value("rl").toArray();
        QStringList sKeys;
        sKeys << "rid" << "rn" << "rs1" << "rs16" << "url" << "nn";
        waitClearRooms();
        for(auto it : arr)
        {
            auto arrObj = it.toObject();
            auto room = new _DyRoom_();
            room->sRid = QString::number(arrObj.value("rid").toInt());
            room->sImg1 = arrObj.value("rs1").toString();
            room->sImg2 = arrObj.value("rs16").toString();
            room->sUrl = arrObj.value("url").toString();
            room->nOnlineCount = arrObj.value("ol").toInt();
            room->sC2Name = arrObj.value("c2name").toString();
            room->sName = arrObj.value("nn").toString();
            room->sRn = arrObj.value("rn").toString();
            room->sOd = arrObj.value("od").toString();
            m_room->append(room);
        }

        emit loadRoomsInfo(m_room->size());
    };

    auto request = new QNetworkRequest();
    request->setUrl(QUrl(QString("https://www.douyu.com") + url + QString::number(nPage)));
    request->setRawHeader("User-Agent", m_sUserAgent.toUtf8());
    appendRequest(request, func);
}

void QDyModel::getLiveArg(const QString &rid)
{
    auto real_rid = rid;
    auto func = [=](QNetworkReply *response)
    {
        QString data = response->readAll();


        _Dy_Sign_Arg_ arg;
        int nPos = 0;
        arg.ver = m_ver;
        arg.v = getAttr(data, "var vdwdae325w_64we = \"", "\"", nPos);
        arg.rid = real_rid;
        arg.tt = QString::number(QDateTime::currentDateTime().toTime_t());
        auto sKey = arg.key();
        auto md5 = QCryptographicHash::hash(sKey.toStdString().c_str(), QCryptographicHash::Md5).toHex().toLower();

        nPos = 0;
        QRegExp re("(function ub9.*)[\\s\\S]return eval");
        re.setMinimal(true);
        re.indexIn(data);
        auto jsCode = re.cap(0);
        jsCode = jsCode.replace("return eval", "return strc;}");
        QRegExp re1("function ub9(.*)[\\(](.*)0,");
        re1.setMinimal(true);
        re1.indexIn(jsCode);
        auto re2Pan = QString("var %1(.*)[\\]];\n").arg(re1.cap(2));
        QRegExp re2(re2Pan);
        re2.setMinimal(true);
        re2.indexIn(data);
        jsCode += re2.cap(0);

        QScriptEngine _engine;
        QScriptValue _js = _engine.evaluate(jsCode);
        QScriptValue func = _engine.globalObject().property("ub98484234");
        QScriptValueList _args;
        auto signFuncValue = func.call(_engine.globalObject(), _args);
        auto oldSignFunc = signFuncValue.toString();
        QRegExp re3("\\(function[\\s\\S]*toString\\(\\)");
        re3.setMinimal(true);
        auto newSignFunc = oldSignFunc.replace(re3, "\'");
        auto signFunc = "function get_sign(){var rb=\'" + md5 + newSignFunc;
        nPos = signFunc.indexOf("return rt;}");
        signFunc = signFunc.left(nPos);
        signFunc += "return re;};";
        QRegExp re5("\"v=.*&sign=\"\\+");
        re5.setMinimal(true);
        signFunc = signFunc.replace(re5, "");
        _js = _engine.evaluate(signFunc);
        func = _engine.globalObject().property("get_sign");
        _args << arg.rid << arg.did << arg.tt;
        auto signValue = func.call(_engine.globalObject(), _args);
        arg.sign = signValue.toString();

//        qDebug() << signFunc;
//        qDebug() << "v:" << arg.v;
//        qDebug() << "did:" << arg.did;
//        qDebug() << "tt:" << arg.tt;
//        qDebug() << "rid:" << arg.rid;
//        qDebug() << "sign:" << arg.sign;
        getLiveUrl(arg);
    };

    auto request = new QNetworkRequest();
    request->setUrl(QUrl(QString("https://m.douyu.com/") + real_rid));
    appendRequest(request, func);
}

void QDyModel::getLiveUrl(const _Dy_Sign_Arg_ &arg)
{
    auto func = [=](QNetworkReply *response)
    {
        QString data = QString::fromUtf8(response->readAll());

        qDebug() << "live" << data.length();
        qDebug() << "live" << data;
        auto doc = QJsonDocument::fromJson(data.toUtf8());
        auto obj = doc.object();
        auto dataObj = obj.value("data").toObject();
        auto url = dataObj["url"].toString();
        QRegExp re("dylive(.*)/(.*).m3u8");
        re.setMinimal(true);
        re.indexIn(url);
        auto list = re.cap(2).split('_');
        auto liveUrl = QString("http://tx2play1.douyucdn.cn/live/%1.flv?uuid=").arg(list[0]);
        qDebug() << liveUrl;
        emit play(liveUrl);
    };

    auto request = new QNetworkRequest();
    request->setUrl(QUrl("https://m.douyu.com/api/room/ratestream"));
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request->setHeader(QNetworkRequest::UserAgentHeader, m_sUserAgent.toUtf8());
    auto data = arg.datas().toUtf8();
    request->setHeader(QNetworkRequest::ContentLengthHeader, data.length());
    appendRequest(request, func, data);
}

void QDyModel::getPreviewUrl(const QString & sRid)
{
    qDebug() << "play->" << sRid;
    auto func = [=](QNetworkReply *response)
    {
        QString data = QString::fromUtf8(response->readAll());

        auto doc = QJsonDocument::fromJson(data.toUtf8());
        auto obj = doc.object();
        if(obj["error"].toInt() != 0)
        {
            getLiveArg(sRid);
            return ;
        }

        auto dataObj = obj.value("data").toObject();
        auto url = dataObj["rtmp_live"].toString();
        auto list = url.split('/');
        auto list1 = list[0].split('_');
        auto liveUrl = QString("http://tx2play1.douyucdn.cn/live/%1.flv?uuid=").arg(list1[0]);
        emit play(liveUrl);
    };

    QString rid = sRid;
    QString tt = QString::number(QDateTime::currentDateTime().toTime_t());
    auto sKey = rid + tt;
    QString md5 = QCryptographicHash::hash(sKey.toUtf8(), QCryptographicHash::Md5).toHex().toLower();
    QString url = QString("https://playweb.douyucdn.cn/lapi/live/hlsH5Preview/%1").arg(rid);
    auto request = new QNetworkRequest();
    request->setUrl(QUrl(url));
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request->setRawHeader("rid", rid.toUtf8());
    request->setRawHeader("time", tt.toUtf8());
    request->setRawHeader("auth", md5.toUtf8());

    auto data = QString("rid=%1&did=10000000000000000000000000001501").arg(rid).toUtf8();
    request->setHeader(QNetworkRequest::ContentLengthHeader, data.length());
    appendRequest(request, func, data);
}

void QDyModel::getVerArg()
{
    auto func = [=](QNetworkReply *response)
    {
        QString data = response->readAll();

        QRegExp re("var n=(.*),a=");
        re.setMinimal(true);
        re.lastIndexIn(data);
        m_ver = re.cap(1);
        emit loadVered();
    };

    auto request = new QNetworkRequest();
    request->setUrl(QUrl(QString("https://shark2.douyucdn.cn/front-publish/live-master/js/player_first/h5_video_96f62c4.js")));
    appendRequest(request, func);
}
