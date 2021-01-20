#ifndef QROOMLISTDELEGATE_H
#define QROOMLISTDELEGATE_H

#include <QStyledItemDelegate>

class QRoomListViewModel;
class QRoomListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    QRoomListDelegate(QObject* parent);
    void setModel(QRoomListViewModel* model);
private:
    void paint(QPainter *painter,
            const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    QRoomListViewModel* m_model;
};

#endif // QROOMLISTDELEGATE_H
