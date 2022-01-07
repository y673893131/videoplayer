#ifndef QNETWORKQUEUE_H
#define QNETWORKQUEUE_H

#include <QObject>
#include <functional>
#include <QList>
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

signals:
    void cleared();
public slots:
    void init();
    void onAppendRequest(QNetworkRequest*, rsp_call, const QByteArray& = QByteArray());
    void clear();
private:
    void threadFunc();
private:
    QNetworkAccessManager* m_net;
    QSemaphore* m_sem;
    QList<_Req_*> m_reqs;
    bool m_bClear;
};

#endif // QNETWORKQUEUE_H
