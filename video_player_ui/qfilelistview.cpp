#include "qfilelistview.h"
#include "qplayfilelistmodel.h"
#include "qfilelistdelegate.h"
#include <QDebug>
QFileListView::QFileListView(QWidget* parent)
    :QListView(parent)
{
    setObjectName("list_file");
    auto model = new QPlayFileListModel(this);
    model->flush();
    auto delegate = new QFileListDelegate(this);
    setModel(model);
    setItemDelegate(delegate);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setMouseTracking(true);
    installEventFilter(this);
}
