#ifndef QNETWORKQUEUE_H
#define QNETWORKQUEUE_H

#include <QObject>
#include <functional>
#include <QVector>
#include <QNetworkRequest>

class QSemaphore;
class QNetworkAccessManager;
class QNetworkReply;
using rsp_call = std::function<void(QNetworkReply*)>;

struct _Req_
{
    _Req_()
        :req(nullptr)
    {
    }

    virtual ~_Req_()
    {
        remove();
    }

    void remove()
    {
        if(req)
        {
            delete req;
            req = nullptr;
        }
    }
    QNetworkRequest* req;
    QByteArray sData;
    rsp_call call;
};


class QNetWorkQueue : public QObject
{
    Q_OBJECT
public:
    explicit QNetWorkQueue(QObject *parent = nullptr);

    void initRspThread();
    void appendRequest(QNetworkRequest*, rsp_call, const QByteArray& = QByteArray());
    void clear();
signals:
    void cleared();
private:
    void threadFunc();
private:
    QNetworkAccessManager* m_net;
    QSemaphore* m_sem;
    QVector<_Req_*> m_reqs;
    bool m_bClear;
};

#endif // QNETWORKQUEUE_H
