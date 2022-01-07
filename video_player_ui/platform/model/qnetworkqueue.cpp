#include "qnetworkqueue.h"
#include <thread>
#include <QThread>
#include <QEventLoop>
#include <QSemaphore>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QDebug>
#include <QMetaType>

QNetWorkQueue::QNetWorkQueue(QObject *parent)
    : QObject(nullptr), m_bClear(false)
{
    m_sem = new QSemaphore();
    auto thread = new QThread();
    moveToThread(thread);
    QTimer::singleShot(0, [=]{init();});
}

void QNetWorkQueue::onAppendRequest(QNetworkRequest *pReq, rsp_call call, const QByteArray &sData)
{
    auto req = new _Req_();
    req->req = pReq;
    req->sData = sData;
    req->call = call;
    m_reqs.push_back(req);
    m_sem->release();
}

void QNetWorkQueue::clear()
{
    if(m_reqs.isEmpty())
    {
        emit cleared();
        return;
    }

    m_bClear = true;
}

void QNetWorkQueue::init()
{
    std::thread([=]{
        threadFunc();
    }).detach();
}

void QNetWorkQueue::threadFunc()
{
    m_net = new QNetworkAccessManager();
    QEventLoop loop;
    connect(m_net, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    while (1) {
        m_sem->acquire();
        auto req = *m_reqs.begin();
        m_reqs.pop_front();
        if(!m_bClear)
        {
            QNetworkReply* reply = nullptr;
            if(req->sData.isEmpty())
                reply = m_net->get(*req->req);
            else
                reply = m_net->post(*req->req, req->sData);
            auto error = reply->error();
//            qDebug() << req->req->url();
//            qDebug() << reply->error();
            if(error != QNetworkReply::NoError)
                qDebug() << reply->errorString() << error;
            loop.exec();
            req->call(reply);
            reply->close();
        }

        delete req;

        if(m_bClear && m_reqs.isEmpty())
        {
            m_bClear = false;
            emit cleared();
        }
    }
}
