#ifndef QLIVEDATAMODEL_H
#define QLIVEDATAMODEL_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QNetworkRequest>
#include <QSemaphore>
#include "qnetworkqueue.h"
#include "croommanager.h"
#include "datadef.h"

class QLiveDataModel : public QObject
{
    Q_OBJECT
public:
    explicit QLiveDataModel(QObject *parent = nullptr);

    void init();
    int classCount();
    QString className(int);
    int gameCount(int);
    QString gameName(int, int);
    QString gameUrl(int, int);
    virtual void loadGameRooms(const QString&);
    virtual void loadGameRoomPage(const QString&, int = 1);
    CRoomManager *roomManager();
protected:
    virtual void initClass();
signals:
    void inited();
    void play(const QString&);
    void totalPage(int, const QString&);
    void loadRoomsInfo(int);
public slots:
    virtual void getPreviewUrl(const QString&);
protected:
    QString getAttr(const QString& data, const QString& attrBegin, const QString& attrEnd, int&);
    void appendRequest(QNetworkRequest*, rsp_call call, const QByteArray& = QByteArray());
    void waitClearRooms();
protected:
    _Dy_ m_dy;
    CRoomManager* m_room;
    QString m_ver;
    QString m_sUserAgent;
    QNetWorkQueue* m_netWorkQueue;
};


#endif // QLIVEDATAMODEL_H
