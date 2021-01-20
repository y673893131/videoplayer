#ifndef QROOMLISTVIEWMODEL_H
#define QROOMLISTVIEWMODEL_H

#include <QAbstractListModel>
#include "croommanager.h"

class QRoomListViewModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum user_role
    {
        rid_role = Qt::UserRole + 1,
        room_info_role,
    };
public:
    explicit QRoomListViewModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex findRoom(const QString&);

    void setRoomManager(CRoomManager*);
public slots:
    void appendRooms(int);
    void onRoomChanged(int n, const QString&, void*)
;private:
    CRoomManager* m_room;
    QSize m_itemSize;
};

#endif // QROOMLISTVIEWMODEL_H
