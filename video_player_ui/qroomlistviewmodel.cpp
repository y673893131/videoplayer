#include "qroomlistviewmodel.h"
#include <QIcon>
#include <QNetworkReply>
#include <QFile>
#include <QApplication>
#include <QDir>
#include <QVector>
#include <QMetaType>
#include <QImage>
#include "framelesswidget/framelesswidget.h"
QRoomListViewModel::QRoomListViewModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_room(nullptr)
{
    m_itemSize = CALC_SIZE(300.0f / 1920, 260.0f / 1080);
}

QVariant QRoomListViewModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
    // FIXME: Implement me!
    return QVariant();
}

int QRoomListViewModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid() || !m_room)
        return 0;

    // FIXME: Implement me!
    return m_room->size();
}

QVariant QRoomListViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_room)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return m_room->index(index.row())->sName;
    case rid_role:
        return m_room->index(index.row())->sRid;
    case room_info_role:
        return QVariant::fromValue(reinterpret_cast<void*>(m_room->index(index.row())));
    case Qt::SizeHintRole:
        return m_itemSize;
    default:
        break;
    }
    return QVariant();
}

QModelIndex QRoomListViewModel::findRoom(const QString &sKey)
{
    auto i = m_room->findRoom(sKey);
    return index(i);
}

void QRoomListViewModel::setRoomManager(CRoomManager *roomManger)
{
    if(m_room)
    {
        emit m_room->clearRooms();
        disconnect(m_room, &CRoomManager::dataChanged, this, &QRoomListViewModel::onRoomChanged);
    }

    m_room = roomManger;
    emit layoutChanged();
    connect(m_room, &CRoomManager::dataChanged, this, &QRoomListViewModel::onRoomChanged);
}

void QRoomListViewModel::appendRooms(int)
{
    emit layoutChanged();
}

void QRoomListViewModel::onRoomChanged(int n, const QString &, void *)
{
    auto cur = index(n);
    emit dataChanged(cur, cur);
}
