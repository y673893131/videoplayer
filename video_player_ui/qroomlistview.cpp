#include "qroomlistview.h"
#include "qroomlistviewmodel.h"
#include "qroomlistdelegate.h"
#include <QScrollBar>
#include <QToolTip>
#include <QMouseEvent>

QRoomListView::QRoomListView(QWidget *parent)
    : QListView(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setObjectName("room_list");
    setViewMode(IconMode);
    setVerticalScrollMode(ScrollPerPixel);
    setHorizontalScrollMode(ScrollPerPixel);
    verticalScrollBar()->setObjectName("scroll1");
    horizontalScrollBar()->setObjectName("scroll1");
    setSpacing(10);

    auto model = new QRoomListViewModel(this);
    setModel(model);
    auto delegate = new QRoomListDelegate(this);
    delegate->setModel(model);
    setItemDelegate(delegate);
}

void QRoomListView::go(const QString &text)
{
    auto model = reinterpret_cast<QRoomListViewModel*>(this->model());
    auto index = model->findRoom(text);
    if(index.isValid())
        scrollTo(index);
}
