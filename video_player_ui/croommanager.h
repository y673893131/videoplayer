#ifndef CROOMMANAGER_H
#define CROOMMANAGER_H
#include <QObject>
#include <QVector>
#include <QPixmap>
#include <QMutex>
#include "qnetworkqueue.h"

//#define ROOM_ICON_SAVE
struct _DyRoom_
{
    _DyRoom_()
        :pm(nullptr)
    {
        nOnlineCount = 0;
        nType = 1;
    }

    virtual ~_DyRoom_()
    {
        clear();
    }

    void clear()
    {
        if(pm)
        {
            delete pm;
            pm = nullptr;
        }
    }
    //room info
    QString sImg1;
    QString sImg2;
    QString sRid;
    QString sUrl;
    QString sLiveUrl;
    int nOnlineCount;
    int nType;
    QString sName;
    QString sRn;
    QString sOd;
    QString sC2Name;
    QPixmap* pm;
};

class CRoomManager : public QObject
{
    Q_OBJECT
public:
    CRoomManager();
    virtual ~CRoomManager();
    void append(_DyRoom_*);
    int size();
    _DyRoom_* index(int n);
    int findRoom(const QString& sKey);
signals:
    void clearRooms();
    void clearRoomsed();
    void dataChanged(int, const QString&, void*);
public slots:
    void clear();
    void flushRooms();
    void onDataChanged(int, const QString&, void*);
private:
    QVector<_DyRoom_*> m_rooms;
    QNetWorkQueue* m_netWorkQueue;
    QString m_iconPath;
    QMutex m_lock;
};

#endif // CROOMMANAGER_H
