#include "qnetworkqueue.h"
#include <thread>
#include <thread>
#include <QEventLoop>
#include <QSemaphore>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>

QNetWorkQueue::QNetWorkQueue(QObject *parent)
    : QObject(parent), m_bClear(false)
{
    m_sem = new QSemaphore();
}

void QNetWorkQueue::appendRequest(QNetworkRequest *pReq, rsp_call call, const QByteArray &sData)
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

void QNetWorkQueue::initRspThread()
{
    std::thread([this]
    {
        threadFunc();
    }).detach();
}

void QNetWorkQueue::threadFunc()
{
    QEventLoop loop;
    m_net = new QNetworkAccessManager();

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
