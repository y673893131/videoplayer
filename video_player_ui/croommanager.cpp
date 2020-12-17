#include "croommanager.h"
#include <QFile>
#include <QApplication>
#include <QNetworkReply>
#include <QDir>

//#define ROOM_ICON_SAVE
CRoomManager::CRoomManager()
{
    m_netWorkQueue = new QNetWorkQueue();
    m_netWorkQueue->initRspThread();
    connect(m_netWorkQueue, &QNetWorkQueue::cleared, this, &CRoomManager::flushRooms, Qt::QueuedConnection);
#ifdef ROOM_ICON_SAVE
    m_iconPath = QApplication::applicationDirPath() + "/temp/img";
    QDir dir;
    if(!dir.exists(m_iconPath))
    {
        dir.mkpath(m_iconPath);
    }
#endif
    connect(this, &CRoomManager::dataChanged, this, &CRoomManager::onDataChanged, Qt::QueuedConnection);
    connect(this, &CRoomManager::clearRooms, this, &CRoomManager::clear, Qt::QueuedConnection);
}

CRoomManager::~CRoomManager()
{

}

void CRoomManager::append(_DyRoom_ *room)
{
    QMutexLocker locker(&m_lock);
    m_rooms.push_back(room);
}

void CRoomManager::clear()
{
    QMutexLocker locker(&m_lock);
    for(auto it : m_rooms)
    {
        delete it;
    }

    m_rooms.clear();
    m_netWorkQueue->clear();
    emit clearRoomsed();
}

int CRoomManager::size()
{
    QMutexLocker locker(&m_lock);
    return m_rooms.size();
}

_DyRoom_ *CRoomManager::index(int n)
{
    QMutexLocker locker(&m_lock);
    if(n < m_rooms.size())
        return m_rooms[n];
    return nullptr;
}

int CRoomManager::findRoom(const QString &sKey)
{
    QMutexLocker locker(&m_lock);
    for(int n = 0; n < m_rooms.size(); ++n)
    {
        if(m_rooms[n]->sName.contains(sKey, Qt::CaseSensitive) ||
            m_rooms[n]->sRid.contains(sKey, Qt::CaseSensitive))
        {
            return n;
        }
    }

    return 0;
}

void CRoomManager::flushRooms()
{
    QMutexLocker locker(&m_lock);
    int n = 0;
    for(auto it : m_rooms)
    {
        auto req = new QNetworkRequest();
        req->setUrl(it->sImg2);
//        qDebug() << it->sImg2;
        auto url = it->sUrl;
        auto rid = it->sRid;
        auto func = [=](QNetworkReply* response)
        {
            auto data = response->readAll();
#ifdef ROOM_ICON_SAVE
            QFile file;
            auto fileName = m_iconPath + url + ".jpg";
            file.setFileName(fileName);
            if(file.open(QIODevice::WriteOnly))
            {
                file.write(data);
                file.close();
            }
#endif
            QImage img = QImage::fromData(data);
//            qDebug() << rid << img.isNull();
            auto pm = new QPixmap(QPixmap::fromImage(img)/*.scaled(305, 220, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)*/);
            emit dataChanged(n, rid, pm);
        };

        m_netWorkQueue->appendRequest(req, func);
        ++n;
    }
}

void CRoomManager::onDataChanged(int index, const QString& rid, void * pm)
{
//    QMutexLocker locker(&m_lock);
    auto room = this->index(index);
    auto pixmap = (QPixmap*)pm;
    if(room && room->sRid == rid)
    {
        room->clear();
        room->pm = pixmap;
    }
}
