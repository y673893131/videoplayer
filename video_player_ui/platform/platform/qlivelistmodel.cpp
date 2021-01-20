#include "qlivelistmodel.h"
#include "platform/model/qdymodel.h"
#include "platform/model/qhuyamodel.h"
#include "platform/model/qegamemodel.h"
#include "platform/room/qroomlistviewmodel.h"
#include <QEventLoop>
QLiveListModel::QLiveListModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_currentType(Live_DouYu)
{
    initTypeData(Live_DouYu, new QDyModel);
    initTypeData(Live_HuYa, new QHuYaModel);
    initTypeData(Live_EGame, new QEGameModel);
}

QVariant QLiveListModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
    return QVariant();
}

int QLiveListModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return curData()->classCount();
    else if (parent.internalId() == -1)
        return curData()->gameCount(parent.row());

    return 0;
}

QVariant QLiveListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        if(!index.parent().isValid())
            return curData()->className(index.row());
        else
            return curData()->gameName(index.parent().row(), index.row());
    case Game_url_role:
        if(index.parent().isValid())
            return curData()->gameUrl(index.parent().row(), index.row());
        break;
    default:
        break;
    }
    // FIXME: Implement me!
    return QVariant();
}


QModelIndex QLiveListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0)
            return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, static_cast<quintptr>(-1));
    return createIndex(row, column, static_cast<quintptr>(parent.row()));
}

QModelIndex QLiveListModel::parent(const QModelIndex &child) const
{
    auto parentRow = static_cast<int>(child.internalId());
    if (!child.isValid() || parentRow == -1)
        return QModelIndex();
    return createIndex(parentRow, 0, static_cast<quintptr>(-1));
}

int QLiveListModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

CRoomManager *QLiveListModel::getRoomManager()
{
    return curData()->roomManager();
}

QLiveDataModel *QLiveListModel::curData() const
{
    return m_data[m_currentType];
}

void QLiveListModel::initTypeData(int index, QLiveDataModel* model)
{
    m_data[index] = model;
    model->init();
    connect(model, &QLiveDataModel::loadRoomsInfo, this, &QLiveListModel::loadRoomsInfo);
    connect(model, &QLiveDataModel::play, this, &QLiveListModel::play);
    connect(model, &QLiveDataModel::totalPage, this, [this](int nPageCount, const QString& sPagePath)
    {
        m_nPageCount = nPageCount;
        m_sPagePath = sPagePath;
        emit resetPage(m_nPageCount);
    });
}

void QLiveListModel::setCurrentType(int type)
{
    auto room = getRoomManager();
    if(room)
    {
        emit room->clearRooms();
        QEventLoop loop;
        connect(room, &CRoomManager::clearRoomsed, &loop, &QEventLoop::quit);
        loop.exec();
    }
    m_currentType = type;
    emit layoutChanged();
}

void QLiveListModel::selectGame(const QModelIndex &index)
{
    if(!index.parent().isValid())
        return;
    auto url = data(index, Game_url_role).toString();
    curData()->loadGameRooms(url);
}

void QLiveListModel::selectRoom(const QModelIndex &index)
{
    auto info = (_DyRoom_*)index.data(QRoomListViewModel::room_info_role).value<void*>();
    if(info->sLiveUrl.isEmpty())
        curData()->getPreviewUrl(info->sRid);
    else
        emit play(info->sLiveUrl);
}

void QLiveListModel::onPageChanged(int page)
{
    curData()->loadGameRoomPage(m_sPagePath, page);
}
